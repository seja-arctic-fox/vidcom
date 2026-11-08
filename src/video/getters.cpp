/*
    VidCom, an utility for video archival and compression
    Copyright (C) 2026  seja-arctic-fox <seja.czstudio@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
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