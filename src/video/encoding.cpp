#include "video.h"
#include "../cli/cli.h"
#include <array>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <format>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <cstring>
#include <unistd.h>
#include <termios.h>

// hlavní metody

Video * Video::current_instance = nullptr;

// Ctrl + C by mělo ukončit všechny ffmpeg procesy, které program spustil
void Video::sigint_handler(int)
{
    if (current_instance)
    {
        pid_t pid = current_instance -> encoding_pid;
        if (pid != -1)
        {
            kill (-pid, SIGKILL);
        }
        
        current_instance -> cancel_encoding();
    }
}

int Video::encode(fs::path output_path, ProgressCallback progress_callback)
{
    cancelling_encoding.store(false);
    encoding_pid = -1;
    float duration;
    vector<string> command = make_options();
    vector<const char *> argv = {"ffmpeg"};
    
    for (const auto& arg : command)
        argv.push_back(arg.c_str());
    argv.push_back(nullptr);

    if (output_path == "")
    {
        output_path = outputPath;
    }

    // vytvořit výstupní složku
    try
    {
        if (fs::create_directory(output_path.parent_path()))
        {
            cout << GREEN << "Creating the output folder: " << output_path.parent_path() << RESET << endl;
        } 
        else 
        {
            cerr << YELLOW << "WARNING: Output folder " << output_path.parent_path() << " already exists! Some files might be overwritten. " << RESET << endl;
        }
    }
    catch (std::filesystem::filesystem_error &e)
    {
        cerr << RED << "ERROR: Cannot write to specified output location! Exiting. " << RESET << endl;
        return -3;      // Znamená, že se nepodařilo složku vytvořit
    }

    // Spuštění příkazu
    char buffer[256];

    int pipe_fds[2];
    int stderr_fds[2];
    
    if (pipe(pipe_fds) != 0)
    {
        return -1;
    }
    
    if (pipe(stderr_fds) != 0)
    {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return -1;
    }

    // Přesměrování výstupu na pipe_fds[1]
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_addclose(&file_actions, pipe_fds[0]);
    posix_spawn_file_actions_addclose(&file_actions, stderr_fds[0]);
    posix_spawn_file_actions_adddup2(&file_actions, pipe_fds[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&file_actions, pipe_fds[1]);
    posix_spawn_file_actions_adddup2(&file_actions, stderr_fds[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&file_actions, stderr_fds[1]);

    // Uložit stav terminálu
    struct termios original_termios;
    bool termios_saved = (tcgetattr(STDIN_FILENO, &original_termios) == 0);

    // Po vytvoření procesu se ten proces automaticky pozastaví pomocí SIGSTOP
    // Musíme to ignorovat
    struct sigaction old_tstp, old_ttin, old_ttou;
    struct sigaction ignore_sa;
    ignore_sa.sa_handler = SIG_IGN;
    sigemptyset(&ignore_sa.sa_mask);
    ignore_sa.sa_flags = 0;
    sigaction(SIGTSTP, &ignore_sa, &old_tstp);
    sigaction(SIGTTIN, &ignore_sa, &old_ttin);
    sigaction(SIGTTOU, &ignore_sa, &old_ttou);

    // Spuštění potomků do skupiny, aby se dali ukončit najednou
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
    posix_spawnattr_setpgroup(&attr, 0);

    // Vytvořit proces
    int spawn_result = posix_spawnp(&encoding_pid, "ffmpeg", &file_actions, &attr, const_cast<char* const*>(argv.data()), nullptr);
    posix_spawnattr_destroy(&attr);
    posix_spawn_file_actions_destroy(&file_actions);

    sigaction(SIGTSTP, &old_tstp, nullptr);
    sigaction(SIGTTIN, &old_ttin, nullptr);
    sigaction(SIGTTOU, &old_ttou, nullptr);

    // Uzavřít rouru pro zapisování, nechat jenom čtení
    close(pipe_fds[1]);
    close(stderr_fds[1]);

    if (spawn_result != 0)
    {
        close(pipe_fds[0]);
        return -1;
    }

    // Napojit Ctrl + C na kill, takže ffmpeg skončí s tímto procesem
    current_instance = this;
    struct sigaction sa;
    struct sigaction old_sa;
    sa.sa_handler = &Video::sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, &old_sa);

    cout << GREEN << "Starting encoding of " << inputVideo.path.filename() << "\n" << RESET << endl;

    // Vytvořit soubor pro výstup roury
    FILE * encoding_pipe = fdopen(pipe_fds[0], "r");
    
    while (fgets(buffer, sizeof(buffer), encoding_pipe) != NULL)
    {
        if (cancelling_encoding.load())
        {
            cout << YELLOW << "\nEncoding cancelled. " << RESET << endl;

            if (encoding_pid != -1)
            {
                kill(-encoding_pid, SIGKILL); // - znamená, že zabijeme celou skupinu, ne jenom ten jeden proces
                waitpid(encoding_pid, nullptr, 0); // sbírá zombie procesy
            }

            fclose(encoding_pipe);
            encoding_pid = -1;
            
            // Obnovit přechozí SIGINT akci
            sigaction(SIGINT, &old_sa, nullptr);
            current_instance = nullptr;
            
            // Obnovit stav terminálu
            if (termios_saved)
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
            }

            return -2;
        }

        if (is_cutting_enabled())
        {
            duration = cut.endTime - cut.startTime;
        }
        else
        {
            duration = inputVideo.duration;
        }

        if (progress_callback)
        {
            parse_progress(buffer, duration, progress_callback);
        }
        else
        {
            CLI::print_encoding_progress(buffer, duration);
        }
    }

    cout << endl;
    fclose(encoding_pipe);

    int child_status = 0;
    waitpid(encoding_pid, &child_status, 0);
    encoding_pid = -1;
    
    FILE * error_pipe = fdopen(stderr_fds[0], "r");
    string error_msg;
    if (error_pipe)
    {
        char err_buffer[256];
        while (fgets(err_buffer, sizeof(err_buffer), error_pipe) != NULL)
            error_msg += err_buffer;
        fclose(error_pipe);
    }
    else
    {
        close(stderr_fds[0]);
    }
    
    if (cancelling_encoding.load())
    {
        sigaction(SIGINT, &old_sa, nullptr);
        current_instance = nullptr;
        if (termios_saved)
            tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        return -2;
    }
    
    int exit_status = WIFEXITED(child_status) ? WEXITSTATUS(child_status) : -1;
    
    if (exit_status != 0) this -> last_error_message = error_msg;

    // Obnovit přechozí SIGINT akci
    sigaction(SIGINT, &old_sa, nullptr);
    current_instance = nullptr;

    // Obnovit stav terminálu
    if (termios_saved)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    }

    cout << YELLOW << "ffmpeg finished with code " << exit_status << RESET << endl;
    cout << GREEN << "Output was saved to: " << outputPath.generic_string() << RESET << endl;

    return exit_status;     // Pro debugging a pro hlášky programu o chybách
}

void Video::parse_progress(char * buffer, float duration, ProgressCallback callback)
{
    string row(buffer);

    // Když nic nenajde, končí se
    if (row.find("out_time_ms") == string::npos)
    {
        return;
    }

    int progress_percent = -1;
    float current_time = 0.0f;
    string value_time = row.substr(12); // Vytáhnout část stringu s číslem, 12 je délka názvu proměnné + =

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
        {"-v", "16", "-y", "-progress", "pipe:1", "-stats_period", "0.1"};
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
        command_params.insert(command_params.end(), 
            {"-c:s", "mov_text"});
    else
        command_params.insert(command_params.end(), 
            {"-c:s", "copy"});

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
        
    cout << "ffmpeg " << command << "\n" << endl;
}
