#ifndef VIDEO_ITEM_H
#define VIDEO_ITEM_H

#include "giomm/settings.h"
#include "glibmm/refptr.h"
#include <json/value.h>
#include <string>
#include <filesystem>
#include <functional>
#include <atomic>

using namespace std;

// Callback funkce pro sledování postupu
using ProgressCallback = std::function<void(float, int)>;

namespace fs = std::filesystem;
extern Glib::RefPtr<Gio::Settings> SETTINGS;

enum Codec // podporované formáty pro kódování videa
{
    AV1,
    VP9,
    HEVC,
    AVC
};

// Parameters for encoders adjustable by user

struct AV1_options
{
    // av1 preset
    short preset = SETTINGS -> get_int("av1-preset");
    // denoising + noise synthesis, film-grain-denoise=1:film-grain=16
    // av1 crf value
    short crf = SETTINGS -> get_int("av1-crf");
    bool film_grain_synthesis = SETTINGS -> get_boolean("av1-fgs");
    // noise level
    short film_grain_level = SETTINGS -> get_int("av1-fgl");
    // overlay frames for better quality
    bool better_details = SETTINGS -> get_boolean("av1-bd");
    // psychovisual tuning for better perceived quality
    bool psychovisual_tuning = SETTINGS -> get_boolean("av1-psy");
    // adaptive bitrate increase based on content
    bool variance_boost = SETTINGS -> get_boolean("av1-vb");

};

struct HEVC_options
{
    // 7 = slower. Values from 0 to 9
    short preset = SETTINGS -> get_int("hevc-preset");
    // hevc crf value
    short crf = SETTINGS -> get_int("hevc-crf");
    // psy-rd=2.5:psy-rdoq=4.0
    bool psychovisual_tuning = SETTINGS -> get_boolean("hevc-psy");
    // merange=100:me=3
    bool motion_estimation = SETTINGS -> get_boolean("hevc-me");
    // aq-mode=4
    bool adaptive_quantisation = SETTINGS -> get_boolean("hevc-aq");
    // bframes=8:b-adapt=2
    bool adaptive_b_frames = SETTINGS -> get_boolean("hevc-ab");
};

struct VP9_options
{
    // 2 = slower. Values from 0 to 7
    short preset = SETTINGS -> get_int("vp9-preset");
    // vp9 crf value
    short crf = SETTINGS -> get_int("vp9-crf");
    // 0 = best, 1 = realtime, 2 = good
    short quality = SETTINGS -> get_int("vp9-qs");
    // 0 = default, 1 = screen (screen recordings), 2 = film
    short tune_content = SETTINGS -> get_int("vp9-tune");
    // Between -8 and 8
    short cpu_used = SETTINGS -> get_int("vp9-cpu");
    // from 0 to 4
    short noise_sensitivity = SETTINGS -> get_int("vp9-ns");
};

struct AVC_options
{
    // = slower. from 0 to 9
    short preset = SETTINGS -> get_int("avc-preset");
    // default, from 0 to 51
    short crf = SETTINGS -> get_int("avc-crf");
    // aq-mode=2
    bool adaptive_quantisation = SETTINGS -> get_boolean("avc-aq");
    // me=umh:subme=9
    bool motion_estimation = SETTINGS -> get_boolean("avc-me");
    // b-adapt=2:bframes=6
    bool adaptive_b_frames = SETTINGS -> get_boolean("avc-ab");
    // 0 = film, 1 = animation, 2 = grain, 3 = stillimage
    short tune = SETTINGS -> get_int("avc-tune");
};

// ------------------------------------------------------------

struct Resolution // rozlišení videa
{
    unsigned int width;
    unsigned int height;
};

struct VideoInfo // informace u vstupním videu
    {
        float duration = -1.0;  // v sekundách. -1 značí, že žádné video nebylo načteno
        Resolution resolution;  // rozlišení v pixelech
        unsigned int framerate; // snímková frekvence
        fs::path path;          // cesta k vstupnímu souboru
        bool use_matroska = false;              // Výchozí kontejner je MP4, v určitých případech bude však lepší MKV
        bool multiple_video_streams = false;    // true, pokud je ve videu více video streamů
    };

struct Cut // informace o střihu
{
    float startTime = 0; 
    float endTime;          // čas začátku a konce kódování, pro jednoduché oříznutí
};

class Video
{
    private: 
        VideoInfo inputVideo;           // informace o vstupním videu
        
        float bitrate;                  // cílový a maximální bitrate
        float maxBitrate;               // oboje v Mbit/s
        float targetSize;               // cílová velikost v MB
        unsigned int outputFPS;         // výstupní snímková frekvence

        Cut cut;                        // střih
        fs::path outputPath;            // místo pro výstup
        string prefix;                  // předpona před souborem
        Codec eCodec;            // volba formátu
        float downscaleFactor;          // Kolikrát zmenšit rozlišení

        bool Compress;                  // pokud je povoleno, zmenšuje, pokud je zakázáno, archivuje. 
        bool TwoPass;                   // dvouprůchodové enkódování
        bool EnableCut;                 // zapne funkci střihu

        // Přerušení kódování
        std::atomic<bool> cancelling_encoding;
        pid_t encoding_pid;
        static Video * current_instance;

        // Metody
        void set_video_info(string input_path);                                         // načte informace o vstupním videu
        string read_video_info(string input_path);                                      // přečte soubor a načte údaje
        void get_video_info_from_json(Json::Value data);                                // vytáhne z json dataframu potřebné údaje
        float get_duration_from_json(Json::Value data, Json::Value video_stream);       // přečte délku videa. Používá tři způsoby
        int get_framerate_from_json(Json::Value video_stream);                          // přečte snímkovou frekvenci. 
        int parse_framerate_fraction(const string& fps_string);                         // ffprobe často ukazuje snímkovou frekvenci jako zlomek. Tato metoda jej přečte
        int get_int_from_json(Json::Value& value, const string& key);                   // získá celé číslo ze vstupu při načítání videa
        void validate_video_info();

        // make_options je souhrná metoda pro tyto (pod)metody
        vector<string> make_options();      // poskládá argumenty podle zvolených nastavení

        // Metody pro jednotlivé kodeky
        vector<string> encode_AV1();         // vytvoří příkaz pro kódování v AV1
        vector<string> encode_HEVC();        // vytvoří příkaz pro kódování v HEVC
        vector<string> encode_VP9();         // vytvoří příkaz pro kódování v VP9
        vector<string> encode_AVC();         // constructs a command for encoding to AVC

        // Sledování postupu
        void parse_progress(char * buffer, float duration, ProgressCallback callback);

    public:
        Video(string input_path);
        ~Video();

        // nastavení kodeků (prozatímní řešení)
        struct AV1_options AV1_options;
        struct HEVC_options HEVC_options;
        struct VP9_options VP9_options;
        struct AVC_options AVC_options;
        
        string last_error_message = "";

        // gettery a settery
        VideoInfo get_video_info();             // vrátí strukturu s informacemi o videu
        Cut get_cut_info();                     // vrátí informace o střihu
        string get_prefix();                    // vrátí předponu před souborem
        string get_output_path();               // vrátí místo, kam se uloží výstup
        int get_output_framerate();             // vrátí snímkovou frekvenci výstupu
        float get_target_bitrate();             // vrátí datový tok výstupu
        Codec get_codec();                      // vrátí kodek pro výstup
        float get_downscale_factor();           // vrátí faktor zmenšení rozlišení
        float get_target_size();                // vrátí cílovou velikost videa

        bool is_compress_enabled();             // komprese/archivace
        bool is_two_pass_enabled();             // dvouprůchodové kódování
        bool is_cutting_enabled();              // střih povolen/zakázán
        
        void set_prefix(string prefix);                    // nastaví předponu souboru
        void set_output_path(string output_path);          // nastaví másto pro uložení výstupu
        void set_output_framerate(unsigned int fps);       // nastavit výstupní snímkovou frekvenci
        void set_bitrate_by_size(float target_size);       // nastaví bitrate pro kompresi
        void set_codec(enum Codec codec);                  // nastavit kodek
        void set_cut(float start_time, float end_time);    // nastaví upravený začátek a konec videa
        void set_downscale_factor(float downscale_factor); // nastaví faktor zmenšení
        void set_default_output_settings();                                             // nastaví výstupní parametry podle vstupních a přiřadí výchozí hodnoty k proměnným
        
        void set_compress(bool compress);                   // nastvit režim
        void set_two_pass(bool two_pass);                   // povolit/zakázat dvouprůchodové kódování
        void enable_cut(bool cut);                          // povolit/zakázat střih

        int encode(fs::path output_path = "", ProgressCallback progress_callback = nullptr); // metoda pro spuštění kódování
        void cancel_encoding();
        static void sigint_handler(int signum);

        void test_commands();     // Testovací metoda pro generované příkazy pro ffmpeg
};

#endif
