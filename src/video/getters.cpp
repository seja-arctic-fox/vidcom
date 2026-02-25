#include "video.h"

// gettery

VideoInfo Video::get_video_info()
{
    return inputVideo;
}

float Video::get_downscale_factor()
{
    return downscaleFactor;
}

string Video::get_output_path()
{
    return outputPath.string();
}

Cut Video::get_cut_info()
{
    return cut;
}

string Video::get_prefix()
{
    return prefix;
}

int Video::get_output_framerate()
{
    return outputFPS;
}

float Video::get_target_bitrate()
{
    return bitrate;
}

Codec Video::get_codec()
{
    return eCodec;
}

bool Video::is_compress_enabled()
{
    return Compress;
}

bool Video::is_two_pass_enabled()
{
    return TwoPass;
}

bool Video::is_cutting_enabled()
{
    return EnableCut;
}

float Video::get_target_size()
{
    return  targetSize;
}