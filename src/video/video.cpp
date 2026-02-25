#include "video.h"
#include "../cli/cli.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <ostream>
#include <stdexcept>
#include <string>

// Konstruktor
Video::Video(string input_path)
:   cancelling_encoding(false)
{
    // Načtení informací o vstupním videu
    set_video_info(input_path);

    // Nastavení výchozích možností
    set_default_output_settings();
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
    string ffprobe_command = "ffprobe -v quiet -print_format json -show_format -show_streams '" + input_path + "'";

    FILE * pipe = popen(ffprobe_command.c_str(), "r"); // otevřít rouru
    cout << GREEN << "Reading information about input file: " << RESET << input_path << endl;
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        json_data += buffer; // čtení dat
    }
    pclose(pipe); // zavřít rouru

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

    // Najdu první video proud v seznamu
    for (int i = 0; i < N; i++)
    {
        Json::Value stream_data = data["streams"][i];
        if (stream_data["codec_type"] == "video")
        {
            video_stream = stream_data;
            video_stream_found = true;
            break;
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