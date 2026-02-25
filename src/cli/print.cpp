#include "cli.h"
#include "../video/video.h"

#include <exception>
#include <iostream>
#include <ostream>
#include <string>

using namespace std;

void CLI::print_input_video_info(VideoInfo video_info)
{
    cout    << "Input video: \n|\t" 
            << video_info.duration 
            << " s\t|\t" 
            << video_info.framerate 
            << " fps\t|\t" 
            << video_info.resolution.width 
            << "x" 
            << video_info.resolution.height 
            << "\t|\n"
            << endl;
}

void CLI::print_encoding_progress(char *stdout_buffer, float duration)
{
    // Vstupní řádek převedeme na string
    string row(stdout_buffer);
    
    // Když nenajde hledanou hodnotu, končí se
    if (row.find("out_time_ms") == string::npos)
    {
        return;
    }
    
    int progress_time;    // Získaná hodnota času, ve kterém se kódování nachází
    cout << "\r";
    string value_string = row.substr(12);       // Vytáhnout část stringu s číslem, 12 je délka názvu proměnné + =

    try 
    {
        cout << "[";
        progress_time = ((stof(value_string) / 1000000) / duration) *  100;     // Spočítáme procenta

        for (int i = 0; i < 100; i++)
        {
            if (i <= progress_time)
            {
                cout << GREEN << "X" << RESET;
            } else {
                cout << YELLOW << "-" << RESET;
            }
        }

        cout << "] " << progress_time << " %"; 
        cout.flush();       // Důležité pro přepisování toho samého řádku. 
    } 
    catch (invalid_argument& e)
    {
        // Očekává se, že se to nepodaří, protože vždy ke konci ffmpeg píše „N/A“
        return;
    }
    return;
}

void CLI::print_exception(exception error)
{
    std::cerr << RED << "An exception occured: " << error.what() << "  It is being handled" << RESET << std::endl;
}

void CLI::print_help()
{
    cout << GREEN << "VidCom 0.8 help page" << RESET << endl;
    cout << "Automatic utility for compressing and archiving videos. This is a CLI interface. You can run the GUI by executing 'vidcom'. \n"
         << "The script features two MODES. COMPRESS compresses the video to a target size and other settings, while ARCHIVE mode (default) makes the video as small as possible without quality sacrifices. \n";
    cout << "____________________" << endl;
    cout << "Usage: \n vidcom [PATH_TO_VIDEO1 PATH_TO_VIDEO2] -Cm/Am -s SIZE_MB -c CODEC -p [PARAMETER1=VALUE1,PARAMETER2=VALUE2] -cut START-STOP -ds FACTOR -fps FPS -2p -o OUTPUT_FOLDER -pr PREFIX -d\n" << endl;
    cout << "Positional arguments \n\t Input videos: \t Paths to video files you want to process. Must be first. Specify as many as you like. \n" << endl;
    cout << "Voluntary arguments (order shouldn't matter)\n\t "
         << "MODES\n"
         << "\t\t-Cm \t Enables COMPRESS mode, which attempts to compress videos to a target size. \n"
         << "\t\t-Am \t Enables ARCHIVE mode, which compresses videos as much as possible without losing quality. (DEFAULT)\n\n"
         << "\t-s SIZE_MB \t Sets target size for output videos if COMPRESS mode is selected. Default is 10 MB\n"
         << "\t-c CODEC \t Sets the codec used for encoding. Codecs are listed below. Default is AV1. \n"
         << "\t-p [PARAMS] \t Changes parameters of chosen codec. Parameters for each codec are listed below. \n"
         << "\t-cut START-STOP  Enables a simple cut feature that trimms video from specified start time to end time. Times are float values in seconds and should be separated by '-'. Disabled by default. \n"
         << "\t-ds FACTOR \t Defines how many times smaller is the output resolution. Expects values larger or equal to 1. Only works in COMPRESS mode. Default is 1, which means it keeps the original resolution. \n"
         << "\t-fps FPS \t Sets the output framerate. Output framerate cannot be larger than the original framerate. Only works in COMPRESS mode. Default is the framerate of the original video. \n"
         << "\t-2p \t\t Enables two-pass encoding and better compression. NOTE: SVTAV1 does not have two-pass encoding in a working state. Disabled by default. \n"
         << "\t-o OUTPUT_PATH \t Sets the output folder where videos are saved. If the path does not end with '/', the last part is used as a prefix for new videos. Default behavior is to save each video in a subfolder created in the parent folder of the video. \n"
         << "\t-pr PREFIX \t Sets the prefix of output videos. Format: PREFIX_ORIGINAL_NAME.mp4. Default is 'C'\n"
         << "\t-d \t\t Prints the ffmpeg command that is being executed. Disables encoding. For debugging purposes. \n"
         << "\nAvaiable codecs: AV1, HEVC, VP9\n"
         << "\tAV1 (libsvtav1) parameters\n"
         << "\t\tp\tint: \t0......13\tEncoding preset. Lower means better compression but longer encoding time. Default: 3\n"
         << "\t\tcrf\tint: \t0......63\tQuality level. Lower values increase quality and bitrate. Default: 35\n"
         << "\t\tfgs\tbool: \ttrue/false\tEnables film grain synthesis postprocessing filter. Default: false\n"
         << "\t\tfgl\tint: \t0......32\tFilm grain synthesis level. Default: 16\n"
         << "\t\tbd\tbool: \ttrue/false\tEnables overlay frames for better details. Default: true\n"
         << "\t\tpt\tbool: \ttrue/false\tEnables psychovisual tuning (better for human eye), instead of PSNR (exact method). Default: true\n"
         << "\t\tvb\tbool: \ttrue/false\tAdaptively increases bitrate when needed. Default: false\n\n"
         << "\tHEVC (libx265) parameters\n"
         << "\t\tp\tint: \t0......9\tEncoding preset. Higher means better compression but longer encoding time. Default: 7\n"
         << "\t\tcrf\tint: \t0......63\tQuality level. Lower values increase quality and bitrate. Default: 19\n"
         << "\t\tme\tbool: \ttrue/false\tEnables better motion estimation. Default: true\n"
         << "\t\tpt\tbool: \ttrue/false\tEnables psychovisual tuning (better for human eye). Default: true\n"
         << "\t\taq\tbool: \ttrue/false\tAdaptive quantisation. Default: true\n"
         << "\t\tab\tbool: \ttrue/false\tAdaptive B-frames. Default: true\n\n"
         << "\tVP9 (libvpx-vp9) parameters\n"
         << "\t\tp\tint: \t0.......7\tEncoding preset. Lower means better compression but longer encoding time. Default: 2\n"
         << "\t\tcrf\tint: \t0......63\tQuality level. Lower values increase quality and bitrate. Default: 23\n"
         << "\t\tq\tint: \t0...1...2\tQuality. 0 = best, 1 = realtime, 2 = good. Default: 0\n"
         << "\t\tt\tint: \t0...1...2\tTune. 0 = default, 1 = screen, 2 = film. Default: 0\n"
         << "\t\tcpu\tint: \t-8......8\tCPU used during encoding. Lower value means longer encoding time. Default: 0\n"
         << "\t\tns\tint: \t0.......4\tNoise sensitivity of the deblocking filter. Should remove compression artifacts. Default: 4\n\n"
         << endl;
}
