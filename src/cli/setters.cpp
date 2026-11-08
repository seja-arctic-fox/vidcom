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
#include "cli.h"
#include <filesystem>

int CLI::set_output_folder(list<Video> &video_list, string output_path)
{
    fs::path path(output_path);
    if (!filesystem::exists(path.parent_path()))
    {
        return 1;
    }

    for (Video &video : video_list)
    {
        video.set_output_path(output_path);
    }

    return 0;
}

void CLI::set_prefix(list<Video> &video_list, string prefix)
{
    for (Video &video : video_list)
    {
        video.set_prefix(prefix);
    }
}

void CLI::set_mode(list<Video> &video_list, bool compress)
{
    for (Video &video : video_list)
    {
        video.set_compress(compress);
    }
}

void CLI::set_2pass(list<Video> &video_list)
{
    for (Video &video : video_list)
    {
        video.set_two_pass(true);
    }
}

int CLI::set_cut(list<Video> &video_list, array<float, 2> timestamps)
{
    float start = timestamps[0];
    float end = timestamps[1];

    if (start > end)
    {
        return 2;
    }

    if (start < 0 || end < 0)
    {
        return 1;
    }

    for (Video &video : video_list)
    {
        if (start > video.get_video_info().duration || end > video.get_video_info().duration)
        {
            return 1;
        }

        video.enable_cut(true);
        video.set_cut(start, end);
    }

    return 0;
}

int CLI::set_downscale(list<Video> &video_list, float downscale)
{
    if (downscale < 1)
    {
        return 1;
    }

    for (Video &video : video_list)
    {
        video.set_downscale_factor(downscale);
    }

    return 0;
}

int CLI::set_fps(list<Video> &video_list, unsigned int fps)
{
    for (Video &video : video_list)
    {
        if (fps > video.get_video_info().framerate)
        {
            return 1;
        }

        video.set_output_framerate(fps);
    }

    return 0;
}

void CLI::set_target_size(list<Video> &video_list, float size_MB)
{
    for (Video &video : video_list)
    {
        video.set_bitrate_by_size(size_MB);
    }
}

void CLI::set_codec(list<Video> &video_list, Codec codec)
{
    for (Video &video : video_list)
    {
        video.set_codec(codec);
    }
}