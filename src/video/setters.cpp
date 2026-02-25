#include "video.h"
#include <filesystem>
#include <iostream>
#include <json/reader.h>
#include <stdexcept>
#include "../cli/cli.h"

// settery

void Video::set_default_output_settings()
{
    set_output_framerate(inputVideo.framerate);
    set_prefix("C");
    set_bitrate_by_size(10);
    set_cut(0, inputVideo.duration);
    set_output_path(inputVideo.path.parent_path());
    set_codec(AV1);
    set_compress(false);
    set_two_pass(false);
    enable_cut(false);
    set_downscale_factor(1);
}

void Video::set_output_path(string output_path)
{
    if (!filesystem::is_directory(output_path))
    {
        fs::path directory(output_path);
        prefix = directory.stem();
        output_path = directory.parent_path();
    }
    outputPath = output_path;
    outputPath /= "encoded_videos/";
    outputPath /= (prefix + "_" + inputVideo.path.filename().generic_string());
}

void Video::set_bitrate_by_size(float target_size)
{
    targetSize = target_size;
    maxBitrate = (target_size / inputVideo.duration) * 8;
    bitrate = maxBitrate * 0.75;
}

void Video::set_downscale_factor(float downscale_factor)
{
    if (downscale_factor >= 1)
    {
        downscaleFactor = downscale_factor;
    }
}

void Video::set_prefix(string prefix)
{
    this->prefix = prefix;
    outputPath = outputPath.parent_path() / (prefix + "_" + inputVideo.path.filename().generic_string());
}

void Video::set_output_framerate(unsigned int fps)
{
    if (fps <= inputVideo.framerate)
    {
        outputFPS = fps;
    }
    else 
    {
        outputFPS = inputVideo.framerate;
    }
}

void Video::set_codec(Codec codec)
{
    eCodec = codec;
}

void Video::set_compress(bool compress)
{
    Compress = compress;
}

void Video::set_two_pass(bool two_pass)
{
    TwoPass = two_pass;
}

void Video::enable_cut(bool cut)
{
    EnableCut = cut;
}

void Video::set_cut(float start_time, float end_time)
{
    if (start_time < 0 || start_time > inputVideo.duration)
    {
        cerr << RED << "Invalid start time of cut. " << RESET << endl;
        return;
    }

    if (end_time < 0 || end_time > inputVideo.duration)
    {
        cerr << RED << "Invalid end time of cut. " << RESET << endl;
        return;
    }

    if (start_time > end_time)
    {
        cerr << RED << "Start time of the cut cannot be greater than the end time. " << RESET << endl;
        return;
    }

    cut.startTime = start_time;
    cut.endTime = end_time;
    return;
}

void Video::set_video_info(string input_path)
{
    inputVideo.path = input_path;
    
    string output(read_video_info(input_path));

    // vytvoření json dataframu
    Json::Value data;
    Json::Reader reader;
    
    if (!reader.parse(output, data))
    {
        cerr << RED << "Failed to parse ffprobe output. " << RESET << endl;
        throw runtime_error("Failed to parse ffprobe output. ");
    }

    try
    {
        // Načíst informace z json dataframu
        get_video_info_from_json(data);
    }
    catch (invalid_argument& error) // Výjimka nastane, když nějaké hodnoty v dataframu nejsou = data ve videu nebyla úplná
    {
        // Ohlásit
        CLI::print_exception(error);
        cerr << RED << "Failed to read video information. Check whether it is a video file. If it is a video file and it is damaged, try remuxing the file with ffmpeg. " << RESET << endl;
        inputVideo.duration = -1;
    }

    CLI::print_input_video_info(inputVideo);
    return;
}