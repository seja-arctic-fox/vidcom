#include "../headers/gui.h"
#include "src/video/video.h"

AV1_Parameters::AV1_Parameters()
:   
    CodecParametersPage(),
    film_grain_synthesis("Film Grain Synthesis", "Enables film grain synthesis postprocessing filter. "),
    film_grain_level("Film Grain Level", "Level of film grain in shot. "),
    better_details("Better Details", "Enables overlay frames for better details. "),
    psychovisual_tuning("Psychovisual Tuning", "Enables psychovisual tuning (better for human eye), instead of PSNR (exact method). "),
    variance_boost("Variance Boost", "Adaptively increases bitrate when needed. Not recommended to use with COMPRESS mode. ")
{
    // encoding_preset.set_digits(0);
    encoding_preset.set_caption("Lower value means better compression but longer encoding time. ");
    encoding_preset.set_range(0, 13);
    
    // crf.set_digits(0);
    crf.set_caption("Quality level. Lower values increase quality and bitrate. ");
    crf.set_range(0, 63);
    
    // film_grain_level.set_digits(0);
    film_grain_level.set_range(0, 32);
    
    // Signály
    encoding_preset.signal_value_changed.connect([this]() 
        { update([this](VideoElement * e) { save_preset(e); }); });
}

AV1_Parameters::~AV1_Parameters()
{}

void AV1_Parameters::load(VideoElement * video_element)
{
    SafeReset safe_reset(is_loading);
    CodecParametersPage::load(video_element);
    AV1_options options = video_element -> video.AV1_options;
    
    encoding_preset.set_value(options.preset);
    crf.set_value(options.crf);
    film_grain_synthesis.set_state(options.film_grain_synthesis);
    film_grain_level.set_value(options.film_grain_level);
    better_details.set_state(options.better_details);
    psychovisual_tuning.set_state(options.psychovisual_tuning);
    variance_boost.set_state(options.variance_boost);
}
