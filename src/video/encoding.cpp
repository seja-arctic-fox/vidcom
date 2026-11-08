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
#include "giomm/datainputstream.h"
#include "giomm/subprocess.h"
#include "glibmm/refptr.h"
#include "video.h"
#include "../cli/cli.h"
#include <array>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <format>
#include <string>
#include <unistd.h>
#include <vector>

// hlavní metody

Video * Video::current_instance = nullptr;

// Ctrl + C by mělo ukončit všechny ffmpeg procesy, které program spustil
void Video::sigint_handler(int)
{
    if (current_instance)
    {
        #ifdef _WIN32
        
        current_instance -> terminate_encoding();
        
        #else
        
        pid_t pid = current_instance -> encoding_pid.load();
        if (pid != -1)
            kill (pid, SIGKILL);
        
        #endif
        
        current_instance -> cancel_encoding();
    }
}

void Video::terminate_encoding()
{
    if (encoding_process)
    {
        encoding_process -> force_exit();
        encoding_process -> wait();
    }
}

int Video::encode(fs::path output_path, ProgressCallback progress_callback)
{
    cancelling_encoding.store(false);
    float duration;
    string buffer;
    vector<string> command = make_options();

    if (output_path == "") { output_path = outputPath; }

    // vytvořit výstupní složku
    try
    {
        if (fs::create_directory(output_path.parent_path()))
        {
            cout << GREEN << "Creating the output folder: " 
                 << output_path.parent_path() << RESET << endl;
        } 
    }
    catch (std::filesystem::filesystem_error &e)
    {
        cerr << RED 
             << "ERROR: Cannot write to specified output location! Exiting. " 
             << RESET << endl;
        return -3;      // Znamená, že se nepodařilo složku vytvořit
    }

    // Spawning the encoding process
    
    try {
        encoding_process = Gio::Subprocess::create(
            command, 
            Gio::Subprocess::Flags::STDOUT_PIPE | 
            Gio::Subprocess::Flags::STDERR_PIPE
        );
    }
    catch (const Glib::Error& e)
    {
        last_error_message = e.what();
        cerr << RED << "Failed to create encoding process!" << RESET << endl;
        return -1;
    }
    encoding_pid = stoi(encoding_process -> get_identifier());
    Glib::RefPtr<Gio::DataInputStream> std_output = Gio::DataInputStream::create
        (encoding_process -> get_stdout_pipe());
    Glib::RefPtr<Gio::DataInputStream> std_error = Gio::DataInputStream::create
        (encoding_process -> get_stderr_pipe());

    // Connect Ctrl + C to sigkill, so ffmpeg ends with this parent process
    current_instance = this;
    auto previous_sigint_handler = signal(SIGINT, &Video::sigint_handler);

    cout << GREEN 
         << "Starting encoding of " 
         << inputVideo.path.filename() 
         << "\n" << RESET << endl;
    
    while (std_output -> read_line(buffer))
    {
        if (cancelling_encoding.load())
        {
            cout << YELLOW << "\nEncoding cancelled. " << RESET << endl;

            if (encoding_process)
                terminate_encoding();

            encoding_pid = -1; 
            std_output -> close();
            std_error -> close();
            signal(SIGINT, previous_sigint_handler);
            current_instance = nullptr;
            return -2;
        }

        if (is_cutting_enabled())
        { duration = cut.endTime - cut.startTime; }
        else
        {  duration = inputVideo.duration; }

        if (progress_callback)
        {  parse_progress(buffer, duration, progress_callback); }
        else
        { CLI::print_encoding_progress(buffer, duration);  }
    }

    cout << endl;

    encoding_process -> wait();
    std_output -> close();
    encoding_pid = -1;
    
    string error_msg;

    string err_buffer;
    while (std_error -> read_line(err_buffer))
        error_msg += err_buffer;
    std_error -> close();
    
    if (cancelling_encoding.load())
    {
        signal(SIGINT, previous_sigint_handler);
        current_instance = nullptr;
        return -2;
    }
    
    int exit_status = encoding_process -> get_if_exited() ?
                      encoding_process -> get_exit_status() : -1;
    
    if (exit_status != 0) this -> last_error_message = error_msg;

    signal(SIGINT, previous_sigint_handler);
    current_instance = nullptr;

    cout << YELLOW << "ffmpeg finished with code " 
         << exit_status << RESET << endl;
    cout << GREEN << "Output was saved to: " << outputPath.string() 
         << RESET << endl;

    return exit_status;     // Pro debugging a pro hlášky programu o chybách
}

void Video::parse_progress(
    string buffer, 
    float duration, 
    ProgressCallback callback
)
{
    // Když nic nenajde, končí se
    if (buffer.find("out_time_ms") == string::npos)
    {
        return;
    }

    int progress_percent = -1;
    float current_time = 0.0f;
    string value_time = buffer.substr(12); 

    try
    {
        current_time = stof(value_time) / 1000000.0f;
        progress_percent = ((current_time / duration) * 100);

        if (callback)
        {
            callback(current_time, progress_percent);
        }
    }
    catch (invalid_argument& e)
    {
        // Očekává se, že se to nepodaří, protože vždy ke konci ffmpeg píše „N/A“
        return;
    }

    return;
}

void Video::cancel_encoding()
{
    cancelling_encoding.store(true);
}

vector<string> Video::make_options()
{
    // Konečný příkaz
    vector<string> command;

    // Základní nastavení ffmpegu
    vector<string> command_prefix = 
        {"ffmpeg", "-nostdin",
         "-v", "16", "-y", "-progress", "pipe:1", "-stats_period", "0.1"};
    // Vstupní soubor
    vector<string> command_input = 
        {"-i", inputVideo.path.string(), "-map", "0:v", "-map", "0:a?", "-map", "0:s?"};
    // Výstupní soubor
    vector<string> command_output = {outputPath.string()};
    // Nastavení bitratu a velikosti při kompresi
    vector<string> command_params;
    // Nastavení použitého kodeku
    vector<string> command_codec;

    // Nastavení limitu bitratu, menšího rozlišení a snímkové frekvence v případě komprese
    if (Compress)
    {
        command_params = 
            {"-fs", format("{}", targetSize) + "M", "-b:v", format("{}", bitrate) + "M"};

        // Pro použití v tomto programu SVT_AV1 nepřijímá nastavení maximálního bitratu
        if (eCodec != AV1)
            command_params.insert(command_params.end(), 
                {"-maxrate", format("{}", maxBitrate) + "M"});

        // Zmenšování rozlišení
        if (downscaleFactor != 1)
        {
            int new_x = inputVideo.resolution.width / downscaleFactor;
            int new_y = inputVideo.resolution.height / downscaleFactor;

            command_params.insert(command_params.end(), 
                {"-s", to_string(new_x) + "x" + to_string(new_y)});
        }

        // Snížení fps
        if (outputFPS != inputVideo.framerate)
            command_params.insert(command_params.end(), 
                {"-r", to_string(outputFPS)});
    }
    
    // Titulky
    if (!inputVideo.use_matroska) 
        command_params.insert(command_params.end(), {"-c:s", "mov_text"});
    else
        command_params.insert(command_params.end(), {"-c:s", "copy"});

    // Nastavení střihu
    if (EnableCut)
    {
        float video_duration = cut.endTime - cut.startTime;
        
        command_params.insert(command_params.end(), 
            {"-t", format("{}", video_duration)});
        command_input.insert(command_input.begin(), 
            {"-ss", format("{}", cut.startTime)});
    }
    
    if (eCodec == AV1)
    {
        command_codec = encode_AV1();
    }
    if (eCodec == HEVC)
    {
        command_codec = encode_HEVC();
    }
    if (eCodec == VP9)
    {
        command_codec = encode_VP9();
    }
    if (eCodec == AVC)
    {
        command_codec = encode_AVC();
    }

    // Konečný příkaz
    // U AV1 se mi ještě nepodařilo vytvořit úspěšný příkaz pro dvouprůchodové kódování
    // NVENC varianty mají implementaci dvou průchodů jinak

    if (TwoPass && eCodec != AV1)
    {}
    // Will be implemented later. VidCom GUI doesn't even use two-pass now
    // {
    //     command =   "ffmpeg " + command_prefix + command_input + command_params + command_codec + "-pass 1 -an -f null /dev/null; "
    //                 +"ffmpeg " + command_prefix + command_input + command_params + command_codec + "-pass 2 " + command_output;
    // }
    else
    {
        command = command_prefix;
        command.insert(command.end(), command_input.begin(), command_input.end());
        command.insert(command.end(), command_params.begin(), command_params.end());
        command.insert(command.end(), command_codec.begin(), command_codec.end());
        command.insert(command.end(), command_output.begin(), command_output.end());
    }
    return command;
}

vector<string> Video::encode_AV1()
{
    vector<string> command_codec;
    string params;

    // U komprimace se nastaví základnější profil, u archivace parametr kvality
    // Při komprimaci zmenšujeme i zvuk pomocí Opus kodeku, při archivaci kopírujeme původní zvuk 
    if (Compress)
        command_codec.insert(command_codec.end(), 
            {"-profile", "main", "-c:a", "libopus"});
    else
        command_codec.insert(command_codec.end(), 
            {"-crf", to_string(AV1_options.crf), "-c:a", "aac", "-q:a", "1"});

    // Nastavit preset, na který se bude kódovat a začít parametry kodeku
    command_codec.insert(command_codec.end(), 
        {"-c:v", "libsvtav1", "-preset", to_string(AV1_options.preset), "-svtav1-params"}
    );

    // Syntéza šumu
    if (AV1_options.film_grain_synthesis)
    {
        params += "film-grain-denoise=1:";
        params += "film-grain=" + to_string(AV1_options.film_grain_level) + ":";
    }

    // tune=0 je lepší pro lidské oko, tune=1 pro strojovou přesnost
    if (AV1_options.psychovisual_tuning)
        params += "tune=0:";
    else
        params += "tune=1:";

    // Vrstvené snímky s přesnějšími detaily
    if (AV1_options.better_details)
        params += "enable_overlays=1:";
    else
        params += "enable_overlays=0:";

    // Adaptivní zvýšení bitratu pro lepší kvalitu
    if (AV1_options.variance_boost)
        params += "enable-variance-boost=1:variance-boost-strength=3:";

    // Pro komprimaci zapnout režim variable bitrate
    if (Compress)
        params += "rc=1:";

    // Další možnosti, jako kvantizační matice, přechody mezi scénami, referenční snímky a rychlejší dekódování
    // 10-bitový formát pixelů je doporučovaný pro přesnost barev a dobře přehratelný
    params += "enable-qm=1:scm=2:lp=60:tile-columns=1:fast-decode=2 -pix_fmt yuv420p10le";

    command_codec.insert(command_codec.end(), {params});
    return command_codec;
}

vector<string> Video::encode_HEVC()
{
    vector<string> command_codec;
    string params;

    // U komprimace se nastaví základnější profil, u archivace parametr kvality
    // Při komprimaci zmenšujeme i zvuk pomocí Opus kodeku, při archivaci kopírujeme původní zvuk 
    if (Compress)
        command_codec.insert(command_codec.end(), 
            {"-profile", "main", "-pix_fmt", "yuv420p", "-c:a", "libopus"});
    else
        command_codec.insert(command_codec.end(), 
            {"-crf", to_string(HEVC_options.crf), "-c:a", "aac", "-q:a", "1"});

    // Nastavit preset, na který se bude kódovat a začít parametry kodeku
    command_codec.insert(command_codec.end(), 
        {"-preset", to_string(HEVC_options.preset), "-c:v", "libx265", "-x265-params"});

    // Psycho-vizuální ladění. Možný nárůst bitratu
    // Dosáhne vyšší kvality pro lidské oko, ale sníží přesnost
    if (HEVC_options.psychovisual_tuning)
    {
        params += "rdoq-level=2:psy-rd=2.5:psy-rdoq=4.0:";
    }
    else {
        params += "rdoq-level=1:";
    }

    // Motion estimation - vyhledávání pohybových vektorů
    if (HEVC_options.motion_estimation)
    {
        params += "me=3:merange=100:";
    }

    // Adaptivní kvantizace s pokročilými možnostmi, jako je lepší přechody barev v tmavých scénách, detekce hran a auto-variance
    if (HEVC_options.adaptive_quantisation)
    {
        params += "aq-mode=4:";
    }

    // Adaptivní B snímky
    if (HEVC_options.adaptive_b_frames)
    {
        params += "b-adapt=2:";
    }

    // Pro archivaci zakázat tyto dva filtry pro lepší zachování šumu, protože tady není problém s bitratem
    if (!Compress)
    {
        params += "no-sao:no-deblock:";
    }

    // Lepší nastavení počtu B snímků
    params += "bframes=8";

    command_codec.insert(command_codec.end(), {params});
    return command_codec;
}

vector<string> Video::encode_VP9()
{
    vector<string> command_codec;

    // U komprimace se nastaví základnější profil, u archivace parametr kvality
    // Při komprimaci zmenšujeme i zvuk pomocí Opus kodeku, při archivaci kopírujeme původní zvuk 
    if (Compress)
        command_codec.insert(command_codec.end(), 
            {"-pix_fmt", "yuv420p", "-c:a", "libopus"});
    else
        command_codec.insert(command_codec.end(), 
            {"-crf", to_string(VP9_options.crf), "-c:a", "aac", "-q:a", "1"});

    // Nastavit preset, na který se bude kódovat a parametry kodeku
    command_codec.insert(command_codec.end(), 
        {
            "-c:v", "libvpx-vp9", "-preset", to_string(VP9_options.preset),
            "-quality", to_string(VP9_options.quality),
            "-tune-content", to_string(VP9_options.tune_content),
            "-cpu-used", to_string(VP9_options.cpu_used),
            "-noise-sensitivity", to_string(VP9_options.noise_sensitivity)
        });

    return command_codec;
}

vector<string> Video::encode_AVC()
{
    vector<string> command_codec;
    
    if (Compress)
        command_codec.insert(command_codec.end(), 
            {"-pix_fmt", "yuv420p", "-c:a", "libopus"});
    else
        command_codec.insert(command_codec.end(), 
            {"-crf", to_string(VP9_options.crf), "-c:a", "aac", "-q:a", "1"});
    
    // set the preset and codec
    command_codec.insert(command_codec.end(), 
        {"-c:v", "libx264", "-preset", to_string(AVC_options.preset)});
    
    array<string, 4> tunes = {"film", "animation", "grain", "stillimage"};
    command_codec.insert(command_codec.end(), 
        {"-tune", tunes[AVC_options.tune]});
    
    string params;
    
    if (AVC_options.motion_estimation)
    {
        params += "me=umh:subme=9:";
    }
    
    if (AVC_options.adaptive_quantisation)
    {
        params += "aq-mode=2:";
    }
    
    if (AVC_options.adaptive_b_frames)
    {
        params += "b-adapt=2:bframes=6:";
    }
    
    params += "ref=4:no-fast-pskip=1:keyint=" 
            + to_string(this -> outputFPS * 10);
    command_codec.insert(command_codec.end(),
        {"-x264-params", params});
    
    return command_codec;
}

void Video::test_commands()
{
    cout << YELLOW << "Debugging parameter was used. The ffmpeg command that is being executed is: " << RESET << endl;
    
    string command;
    for (auto c : make_options())
        command += c + " ";
        
    cout << command << "\n" << endl;
}
