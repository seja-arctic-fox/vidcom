#ifndef CLI_H
#define CLI_H

#include "../video/video.h"
#include <exception>
#include <list>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"

class CLI
{
    public:
        static void print_input_video_info(VideoInfo video_info);
        static void print_encoding_progress(char * stdout_buffer, float original_duration);
        static int parse_arguments(int argc, char **argv);
        static void print_exception(exception error);
        static void print_help();
        static int set_codec_parameters(list<Video> &video_list, string params);
        static int encode_videos(list<Video> &video_list);
        static void set_mode(list<Video> &video_list, bool compress);
        static void set_codec(list<Video> &video_list, Codec codec);
        static int set_output_folder(list<Video> &video_list, string output_path);
        static void set_target_size(list<Video> &video_list, float size_MB);
        static int set_cut(list<Video> &video_list, array<float, 2> timestamps);
        static int set_downscale(list<Video> &video_list, float downscale);
        static int set_fps(list<Video> &video_list, unsigned int fps);
        static void set_2pass(list<Video> &video_list);
        static void set_prefix(list<Video> &video_list, string prefix);
};

#endif