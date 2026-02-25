#include "cli.h"
#include <cstddef>
#include <iostream>

bool stob(string s)
{
    if (s == "true" || s == "True" || s == "1")
    {
        return true;
    }
    return false;
}

int CLI::set_codec_parameters(list<Video> &video_list, string params)
{
    list<string> parameter_list;
    size_t pos;
    string sliceby = ",";
    bool valid;

    while ((pos = params.find(",")) != std::string::npos) {
        parameter_list.push_back(params.substr(0, pos)); 
        params.erase(0, pos + sliceby.length()); 
    }
    parameter_list.push_back(params);

    Codec codec = video_list.front().get_codec();

    if (codec == AV1)
    {
        AV1_options new_options;

        for (string p : parameter_list)
        {
            string option = p.substr(0, p.find("="));
            string value = p.substr(p.find("=") + 1, p.length());

            if (option == "fgs")
            {
                new_options.film_grain_synthesis = stob(value);
                continue;
            }

            if (option == "fgl")
            {
                new_options.film_grain_level = stoi(value);
                continue;
            }

            if (option == "p")
            {
                new_options.preset = stoi(value);
                continue;
            }

            if (option == "bd")
            {
                new_options.better_details = stob(value);
                continue;
            }

            if (option == "pt")
            {
                new_options.psychovisual_tuning = stob(value);
                continue;
            }

            if (option == "vb")
            {
                new_options.variance_boost = stob(value);
                continue;
            }

            if (option == "crf")
            {
                new_options.crf = stoi(value);
                continue;
            }

            return 2;
        }

        valid = 
            (0 <= new_options.film_grain_level  && new_options.film_grain_level <= 32) ||
            (0 <= new_options.preset            && new_options.preset           <= 13) ||
            (0 <= new_options.crf               && new_options.crf              <= 63) ;

        if (valid)
        {
            for (Video &video : video_list)
            {
                video.AV1_options = new_options;
            }
        }
        else 
        {
            return 1;
        }

    }

    if (codec == HEVC)
    {
        HEVC_options new_options;

        for (string p : parameter_list)
        {
            string option = p.substr(0, p.find("="));
            string value = p.substr(p.find("=") + 1, p.length());

            if (option == "p")
            {
                new_options.preset = stoi(value);
                continue;
            }

            if (option == "crf")
            {
                new_options.crf = stoi(value);
                continue;
            }

            if (option == "pt")
            {
                new_options.psychovisual_tuning = stob(value);
                continue;
            }

            if (option == "me")
            {
                new_options.motion_estimation = stob(value);
                continue;
            }

            if (option == "aq")
            {
                new_options.adaptive_quantisation = stob(value);
                continue;
            }

            if (option == "ab")
            {
                new_options.adaptive_b_frames = stob(value);
                continue;
            }

            return 2;

        }

        valid = 
            (0 <= new_options.preset            && new_options.preset           <=  9) ||
            (0 <= new_options.crf               && new_options.crf              <= 63) ;

        if (valid)
        {
            for (Video &video : video_list)
            {
                video.HEVC_options = new_options;
            }
        }
        else 
        {
            return 1;
        }
    }

    if (codec == VP9)
    {
        VP9_options new_options;

        for (string p : parameter_list)
        {
            string option = p.substr(0, p.find("="));
            string value = p.substr(p.find("=") + 1, p.length());

            if (option == "p")
            {
                new_options.preset = stoi(value);
                continue;
            }

            if (option == "crf")
            {
                new_options.crf = stoi(value);
                continue;
            }

            if (option == "q")
            {
                new_options.quality = stoi(value);
                continue;
            }

            if (option == "t")
            {
                new_options.tune_content = stoi(value);
                continue;
            }

            if (option == "cpu")
            {
                new_options.cpu_used = stoi(value);
                continue;
            }

            if (option == "ns")
            {
                new_options.noise_sensitivity = stoi(value);
                continue;
            }

            return 2;
        }

        valid = 
            (0 <= new_options.crf               && new_options.crf              <= 63) ||
            (0 <= new_options.preset            && new_options.preset           <=  7) ||
            (0 <= new_options.quality           && new_options.quality          <=  2) ||
            (0 <= new_options.tune_content      && new_options.tune_content     <=  2) ||
            (-8<= new_options.cpu_used          && new_options.cpu_used         <=  8) ||
            (0 <= new_options.noise_sensitivity && new_options.noise_sensitivity<=  8) ;      

        if (valid)
        {
            for (Video &video : video_list)
            {
                video.VP9_options = new_options;
            }
        }
        else 
        {
            return 1;
        }

    }
    return 0;
}

int CLI::parse_arguments(int argc, char **argv)
{
    bool debug = false;
    list<Video> * video_list = new list<Video>; // Seznam video objektů ke zpracování
    
    for (int a = 1; a < argc; a++)
    {
        string command(argv[a]);

        if (argv[a][0] == '-')
        {
            // Argumenty a nastavení

            // Pomoc
            if (command == "-h")
            {
                print_help();
                delete video_list;
                return 0;
            }

            // Pro operace na nastavení kódování
            if (!video_list->empty())
            {
                // Režim
                if (command == "-Cm")
                {
                    set_mode(*video_list, true);
                    continue;
                }
                else if (command == "-Am") 
                {
                    set_mode(*video_list, false);
                    continue;
                }

                // Zapnutí dvouprůchodu
                if (command == "-2p")
                {
                    set_2pass(*video_list);
                }

                // Debug ffmpeg příkazu
                if (command == "-d")
                {
                    debug = true;
                }

                // Nastavení cílové velikosti
                if (command == "-s")
                {
                    a++;
                    command = argv[a];

                    set_target_size(*video_list, stof(command));
                    continue;
                }

                // Nastavení kodeku
                if (command == "-c")
                {
                    a++;
                    command = argv[a];

                    if (command == "AV1" || command == "av1")
                    {
                        set_codec(*video_list, AV1);
                    }
                    else if (command == "VP9" || command == "vp9") 
                    {
                        set_codec(*video_list, VP9);
                    }
                    else if (command == "HEVC" || command == "hevc") 
                    {
                        set_codec(*video_list, HEVC);
                    }
                    else 
                    {
                        cerr << RED << "Invalid codec name. Please use -h to list avaiable codecs. " << RESET << endl;
                    }

                    continue;
                }

                // Nastavení parametrů
                if (command == "-p")
                {
                    a++;
                    command = argv[a];

                    set_codec_parameters(*video_list, command);
                    continue;
                }

                // Nastavení jednotného výstupu pro všechna videa
                if (command == "-o")
                {
                    a++;
                    command = argv[a];

                    switch (set_output_folder(*video_list, command))
                    {
                        case 0:
                            break;
                        case 1:
                            cerr << RED << "Specified output path does not exist. " << RESET << endl;
                            delete video_list;
                            return 1;
                    }

                    continue;
                }

                // Nastavení prefixu pro každá videa
                if (command == "-pr")
                {
                    a++;
                    command = argv[a];

                    set_prefix(*video_list, command);
                    
                    continue;
                }

                // Nastavení střihu
                if (command == "-cut")
                {
                    array<float, 2> timestamps;
                    a++;
                    command = argv[a];

                    int pos = command.find("-");

                    if (pos != -1)
                    {
                        timestamps[0] = stof(command.substr(0, pos));
                        timestamps[1] = stof(command.substr(pos + 1, command.length()));
                    }
                    else 
                    {
                        cerr << RED << "Not enough values for setting cut. " << RESET << endl;
                        delete video_list;
                        return 1;
                    }

                    switch (set_cut(*video_list, timestamps))
                    {
                        case 0:
                            break;
                        case 1:
                            cerr << RED << "Invalid values for setting cut. Make sure they are positive and not larger than the original video duration. " << RESET << endl;
                            delete video_list;
                            return 1;
                        case 2: 
                            cerr << RED << "Start time of the cut is larger than the end time" << RESET << endl;
                            delete video_list;
                            return 1;
                    }

                    continue;
                }

                // Nastavení downscale
                if (command == "-ds")
                {
                    a++;
                    command = argv[a];

                    if (set_downscale(*video_list, stof(command)) == 1)
                    {
                        cerr << RED << "Downscale factor must be a number greater or equal to 1. " << RESET << RESET << endl;
                        delete video_list;
                        return 1;
                    }
                    continue;
                }

                // Nastavení nižších fps
                if (command == "-fps")
                {
                    a++;
                    command = argv[a];

                    unsigned int fps = stoi(command);

                    if (set_fps(*video_list, fps) == 1)
                    {
                        cerr << RED << "Output framerate cannot be larger than the original framerate" << RESET << endl;
                        delete video_list;
                        return 1;
                    }

                    continue;
                }
            } 
            else 
            {
                cerr << RED << "First specify at least one input file. " << RESET << endl;
                delete video_list;
                return 1;
            }
        }
        else 
        {   
            if (fs::exists(command))
            {
                video_list -> emplace_front(command);
            } 
            else 
            {
                cerr << RED << "Invalid input path/argument. See -h for CLI usage. " << RESET << endl;
                delete video_list;
                return 1;
            }
        }
    }

    for (Video &video : *video_list)
    {
        if (debug)
        {
            video.test_commands();
        }
        else
        {
            if (video.encode() != 0)
            {
                cerr << RED << "ENCODING FAILED!" << RESET << endl;
            }
            else 
            {
                cout << GREEN << "ENCODING FINISHED!" << RESET << endl;
            }
        }
    }

    delete video_list;
    return 0;
}