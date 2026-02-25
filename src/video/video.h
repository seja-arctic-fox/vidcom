#ifndef VIDEO_ITEM_H
#define VIDEO_ITEM_H

#include <json/value.h>
#include <string>
#include <filesystem>
#include <functional>
#include <atomic>

using namespace std;

// Callback funkce pro sledování postupu
using ProgressCallback = std::function<void(float, int)>;

namespace fs = std::filesystem;

enum Codec // podporované formáty pro kódování videa
{
    AV1,
    VP9,
    HEVC
};

// Možnosti pro enkodéry, které mají být nastavitelné uživatelem

struct AV1_options
{
    bool film_grain_synthesis = false;  // odšumení + syntéza šumu, film-grain-denoise=1:film-grain=16
    short film_grain_level = 16;        // úroveň šumu
    short preset = 3;                   // vysoká úroveň komprese za přijatelný čas
    bool better_details = true;         // enable_overlays
    bool psychovisual_tuning = true;    // tune=0
    short crf = 35;                     // výchozí úroveň kvality
    bool variance_boost = false;        // adaptivní zvýšení bitratu

};

struct HEVC_options
{
    short preset = 7;                   // = slower. Hodnoty od 0 do 9
    short crf = 19;                     // dobrá kvalita i komprese
    bool psychovisual_tuning = true;    // psy-rd=2.5:psy-rdoq=4.0
    bool motion_estimation = true;      // merange=100:me=3
    bool adaptive_quantisation = true;  // aq-mode=4
    bool adaptive_b_frames = true;      // bframes=8:b-adapt=2
};

struct VP9_options
{
    short preset = 2;               // = slower. Hodnoty od 0 do 7
    short quality = 0;              // 0 = best, 1 = realtime, 2 = good
    short tune_content = 0;         // 0 = default, 1 = screen (záznamy obrazovky), 2 = film
    short cpu_used = 0;             // Mezi -8 a 8
    short noise_sensitivity = 4;    // od 0 do 4
    short crf = 23;                 // ideální kompromis
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
        void set_default_output_settings();                                             // nastaví výstupní parametry podle vstupních a přiřadí výchozí hodnoty k proměnným
        string read_video_info(string input_path);                                      // přečte soubor a načte údaje
        void get_video_info_from_json(Json::Value data);                                // vytáhne z json dataframu potřebné údaje
        float get_duration_from_json(Json::Value data, Json::Value video_stream);       // přečte délku videa. Používá tři způsoby
        int get_framerate_from_json(Json::Value video_stream);                          // přečte snímkovou frekvenci. 
        int parse_framerate_fraction(const string& fps_string);                         // ffprobe často ukazuje snímkovou frekvenci jako zlomek. Tato metoda jej přečte
        int get_int_from_json(Json::Value& value, const string& key);                   // získá celé číslo ze vstupu při načítání videa
        void validate_video_info();

        // make_options je souhrná metoda pro tyto (pod)metody
        string make_options();      // poskládá argumenty podle zvolených nastavení

        // Metody pro jednotlivé kodeky
        string encode_AV1();         // vytvoří příkaz pro kódování v AV1
        string encode_HEVC();        // vytvoří příkaz pro kódování v HEVC
        string encode_VP9();         // vytvoří příkaz pro kódování v VP9

        // Sledování postupu
        void parse_progress(char * buffer, float duration, ProgressCallback callback);

    public:
        Video(string input_path);
        ~Video();

        // nastavení kodeků (prozatímní řešení)
        struct AV1_options AV1_options;
        struct HEVC_options HEVC_options;
        struct VP9_options VP9_options;

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

        void set_compress(bool compress);                   // nastvit režim
        void set_two_pass(bool two_pass);                   // povolit/zakázat dvouprůchodové kódování
        void enable_cut(bool cut);                          // povolit/zakázat střih

        int encode(fs::path output_path = "", string command = "", ProgressCallback progress_callback = nullptr); // metoda pro spuštění kódování
        void cancel_encoding();
        static void sigint_handler(int signum);

        void test_commands();     // Testovací metoda pro generované příkazy pro ffmpeg
};

#endif