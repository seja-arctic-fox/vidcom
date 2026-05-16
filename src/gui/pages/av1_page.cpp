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
    encoding_preset.set_caption("Lower value means better compression but longer encoding time. ");
    crf.set_caption("Quality level. Lower values increase quality and bitrate. ");
    
    encoding_preset.set_range(0, 13);
    film_grain_level.set_digits(0);
    film_grain_level.set_adjustment(16, 0, 32, 1);
    
    append(film_grain_synthesis);
    append(film_grain_level);
    append(better_details);
    append(psychovisual_tuning);
    append(variance_boost);
    
    // Signály
    film_grain_synthesis.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_film_grain(e); }); });
    film_grain_level.signal_value_changed.connect([this]()
        { update([this](VideoElement * e){ save_film_grain(e); }); });
    better_details.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_better_details(e); }); });
    psychovisual_tuning.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_psychovisual_tuning(e); }); });
    variance_boost.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_variance_boost(e); }); });
}

void AV1_Parameters::load(VideoElement * video_element)
{
    SafeReset safe_reset(is_loading);
    CodecParametersPage::load(video_element);
    AV1_options options = video_element -> video.AV1_options;
    
    encoding_preset.set_value(options.preset);
    crf.set_value(options.crf);
    film_grain_synthesis.set_state(options.film_grain_synthesis);
    film_grain_level.set_value(options.film_grain_level);
    film_grain_level.set_sensitive(options.film_grain_synthesis);
    better_details.set_state(options.better_details);
    psychovisual_tuning.set_state(options.psychovisual_tuning);
    variance_boost.set_state(options.variance_boost);
}

void AV1_Parameters::save_film_grain(VideoElement * element)
{
    element -> video.AV1_options.film_grain_synthesis = film_grain_synthesis.get_state();
    film_grain_level.set_sensitive(element -> video.AV1_options.film_grain_synthesis);
    element -> video.AV1_options.film_grain_level = film_grain_level.get_value();
}

void AV1_Parameters::save_better_details(VideoElement * element)
{ element->video.AV1_options.better_details = better_details.get_state(); }

void AV1_Parameters::save_psychovisual_tuning(VideoElement * element)
{ element->video.AV1_options.psychovisual_tuning = psychovisual_tuning.get_state(); }

void AV1_Parameters::save_variance_boost(VideoElement * element)
{ element->video.AV1_options.variance_boost = variance_boost.get_state(); }