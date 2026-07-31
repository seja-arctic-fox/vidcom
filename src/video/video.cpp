#include "video.h"
#include "../cli/cli.h"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <ostream>
#include <spawn.h>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

// Konstruktor
Video::Video(string input_path)
:   
    prefix("C"),
    eCodec(AV1),
    downscaleFactor(1),
    Compress(SETTINGS -> get_boolean("encoding-mode")),
    TwoPass(false), 
    EnableCut(false),
    cancelling_encoding(false)
{
    // Načtení informací o vstupním videu
    if (input_path == "")
        inputVideo.path = fs::path(getenv("HOME")) / "example_video.mp4";
    else
        set_video_info(input_path);

    // Nastavení výchozích možností
    set_output_framerate(inputVideo.framerate);
    set_bitrate_by_size(10);
    set_cut(0, inputVideo.duration);
    
    bool next_to_original = SETTINGS -> get_boolean("output-next-to-original");
    if (next_to_original)
        set_output_path(inputVideo.path.parent_path());
    else
        set_output_path(SETTINGS -> get_string("output-path"));
}

// Destruktor
Video::~Video()
{}

// získání informací o vstupu metody

string Video::read_video_info(string input_path)
{
    // příkaz pro načtení a string proměnná pro uložení výstupu
    char buffer[128];
    string json_data = "";
    vector<const char *> command = 
        {
            "ffprobe", "-v", "quiet", "-print_format", "json", 
            "-show_format", "-show_streams", input_path.c_str(), nullptr
        };
    
    int pipe_fds[2];
    if (pipe(pipe_fds) != 0)
    {
        cout << RED << "Failed to execute ffprobe!" << RESET << endl;
        return json_data;
    }
    
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_adddup2(&file_actions, pipe_fds[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&file_actions, pipe_fds[0]);
    posix_spawn_file_actions_addclose(&file_actions, pipe_fds[1]);
    
    pid_t ffprobe_pid;
    int spawn_result = posix_spawnp(&ffprobe_pid, "ffprobe", &file_actions, nullptr, 
                                    const_cast<char* const*>(command.data()), nullptr);
    posix_spawn_file_actions_destroy(&file_actions);
    close(pipe_fds[1]);
    
    if (spawn_result != 0)
    {
        close(pipe_fds[0]);
        cout << RED << "Failed to execute ffprobe!" << RESET << endl;
        return json_data;
    }
    
    cout << GREEN << "Reading information about input file: " << RESET << input_path << endl;
    
    FILE * pipe = fdopen(pipe_fds[0], "r"); // otevřít rouru
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        json_data += buffer; // čtení dat
    }
    fclose(pipe); // zavřít rouru
    int status;
    waitpid(ffprobe_pid, &status, 0);

    return json_data;
}

void Video::get_video_info_from_json(Json::Value data)
{
    // počet položek v arrayi „streams“
    int N = data["streams"].size();

    // Když nemá žádné proudy, není to video
    if (N == 0)
    {
        cerr << RED << "Input file is not a video file or no streams found. " << RESET << endl;
        throw invalid_argument("No streams found in input file. ");
    }

    Json::Value video_stream;
    bool video_stream_found = false;
    int num_video_streams = 0;

    // Najdu první video proud v seznamu
    for (int i = 0; i < N; i++)
    {
        Json::Value stream_data = data["streams"][i];
        if (stream_data["codec_type"] == "video")
        {
            if (!video_stream_found)
            {
                video_stream = stream_data;
                video_stream_found = true;
            }
            
            num_video_streams++;
        }
        
        if (stream_data["codec_type"] == "subtitle")
        {
            auto subtitle_codec = stream_data["codec_name"];
            bool subtitles_compatible = subtitle_codec == "subrip" ||
                                        subtitle_codec == "webvtt" ||
                                        subtitle_codec == "text"   ||
                                        subtitle_codec == "mov_text";
            
            if (!subtitles_compatible) { inputVideo.use_matroska = true; }
        }
    }

    // Končíme, pokud nebyl nalezen žádný video proud
    if (!video_stream_found)
    {
        cerr << RED << "No video stream found!" << RESET << endl;
        throw invalid_argument("No video stream found");
    }

    // Načítání údajů
    inputVideo.duration = get_duration_from_json(data, video_stream);
    inputVideo.framerate = get_framerate_from_json(video_stream);
    inputVideo.resolution.width = get_int_from_json(video_stream, "width");
    inputVideo.resolution.height = get_int_from_json(video_stream, "height");
    validate_video_info();
    
    if (num_video_streams > 1) 
    {
        inputVideo.use_matroska = true; 
        inputVideo.multiple_video_streams = true; 
        cout << YELLOW << "WARNING: Multiple video streams detected!" << RESET << endl;
    }

    return;
}

float Video::get_duration_from_json(Json::Value data, Json::Value video_stream)
{
    float duration = -1;

    // Buď je to v proudu
    if (video_stream.isMember("duration") && !video_stream["duration"].isNull())
    {
        try
        {
            duration = stof(video_stream["duration"].asString());

            if (duration > 0)
            {
                return duration;
            }
        }
        catch (...)
        {}
    }

    // Nebo je to v datech formátu
    if (data.isMember("format") && data["format"].isMember("duration"))
    {
        try 
        {
            duration = stof(data["format"]["duration"].asString());

            if (duration > 0)
            {
                return duration;
            }
        }
        catch (...)
        {}
    }

    // V nejhorším případě lze vypočítat dobu trvání z počtu snímků
    if (video_stream.isMember("nb_frames") && !video_stream["nb_frames"].isNull())
    {
        try
        {
            int nb_frames = stoi(video_stream["nb_frames"].asString());
            float fps = get_framerate_from_json(video_stream);

            if (nb_frames > 0 && fps > 0)
            {
                duration = nb_frames / fps;
                return duration;
            }
        }
        catch (...)
        {}
    }

    throw invalid_argument("Could not determine video duration");
}

int Video::get_framerate_from_json(Json::Value video_stream)
{
    int framerate = 0;
    
    // Získání fps z hodnoty průměrné fps (kvůli variabilním fps)
    if (video_stream.isMember("avg_frame_rate") && !video_stream["avg_frame_rate"].isNull())
    {
        string avg_fps_str = video_stream["avg_frame_rate"].asString();
        framerate = parse_framerate_fraction(avg_fps_str);

        if (framerate > 0 && framerate < 1000) 
        {   
            return framerate;
        }
    }
    
    // Nebo získání klasické hodnoty fps
    if (video_stream.isMember("r_frame_rate") && !video_stream["r_frame_rate"].isNull())
    {
        string r_fps_str = video_stream["r_frame_rate"].asString();
        framerate = parse_framerate_fraction(r_fps_str);

        if (framerate > 0 && framerate < 1000) 
        {
            return framerate;
        } 
    }
    
    // Vypočítat z celkového počtu snímků a doby trvání
    if (video_stream.isMember("nb_frames") && video_stream.isMember("duration"))
    {
        try 
        {
            int nb_frames = stoi(video_stream["nb_frames"].asString());
            float duration = stof(video_stream["duration"].asString());

            if (nb_frames > 0 && duration > 0) 
            {
                framerate = round(nb_frames / duration);
                return framerate;
            }
        } 
        catch (...) {}
    }
    
    throw invalid_argument("Could not determine video framerate");
}

int Video::parse_framerate_fraction(const string& fps_string)
{
    size_t slash_pos = fps_string.find('/');
    
    if (slash_pos != string::npos)
    {
        try 
        {
            int numerator = stoi(fps_string.substr(0, slash_pos));
            int denominator = stoi(fps_string.substr(slash_pos + 1));
            
            if (denominator > 0) 
            {
                return round((float)numerator / denominator);
            }
        } catch (...) 
        {
            return 0;
        }
    }
    else
    {
        try 
        {
            return round(stof(fps_string));
        } 
        catch (...) 
        {
            return 0;
        }
    }
    
    return 0;
}

int Video::get_int_from_json(Json::Value& value, const string& key)
{
    if (!value.isMember(key) || value[key].isNull())
    {
        throw invalid_argument("Missing required field: " + key);
    }
    
    try 
    {
        if (value[key].isInt()) 
        {
            return value[key].asInt();
        } 
        else 
        {
            return stoi(value[key].asString());
        }
    } catch (...) 
    {
        throw invalid_argument("Invalid value for field: " + key);
    }
}

void Video::validate_video_info()
{
    bool valid = true;
    
    if (inputVideo.duration <= 0)
    {
        cerr << RED << "ERROR: Invalid duration (" << inputVideo.duration << " s)" << RESET << endl;
        valid = false;
    }
    
    if (inputVideo.framerate <= 0 || inputVideo.framerate > 500)
    {
        cerr << RED << "ERROR: Invalid framerate (" << inputVideo.framerate << " fps)" << RESET << endl;
        valid = false;
    }
    
    if (inputVideo.resolution.width <= 0 || inputVideo.resolution.height <= 0)
    {
        cerr << RED << "ERROR: Invalid resolution (" 
             << inputVideo.resolution.width << "x" << inputVideo.resolution.height << ")" << RESET << endl;
        valid = false;
    }
    
    if (!valid)
    {
        throw invalid_argument("Video information validation failed");
    }
}
