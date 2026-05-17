#include "../headers/gui.h"

VP9_Parameters::VP9_Parameters()
:
    CodecParametersPage(),
    cpu_usage("CPU Usage", "Amount of CPU used during encoding. Lower values mean better compression but longer encoding time. "),
    noise_sensitivity("Noise Sensitivity", "Sensitivity of the deblocking filter which removes encoding artifacts. "),
    quality_scale("Quality Scale", "0 = best, 1 = realtime, 2 = good"),
    tune("Tune", "0 = default, 1 = screen recording, 2 = film content")
{
    encoding_preset.set_caption("Lower value means better compression but longer encoding time. ");
    crf.set_caption("Quality level. Lower values increase quality and bitrate. ");
    
    encoding_preset.set_range(0, 9);
    cpu_usage.set_digits(0);
    cpu_usage.set_adjustment(0, -8, 8, 1);
    noise_sensitivity.set_digits(0);
    noise_sensitivity.set_adjustment(4, 0, 4, 1);
    quality_scale.set_digits(0);
    quality_scale.set_adjustment(0, 0, 2, 1);
    tune.set_digits(0);
    tune.set_adjustment(0, 0, 2, 1);
    
    append(cpu_usage);
    append(noise_sensitivity);
    append(quality_scale);
    append(tune);
    
    // Signály
    cpu_usage.signal_value_changed.connect([this]()
        { update([this](VideoElement * e){ save_cpu_usage(e); }); });
    noise_sensitivity.signal_value_changed.connect([this]()
        { update([this](VideoElement * e){ save_noise_sensitivity(e); }); });
    quality_scale.signal_value_changed.connect([this]()
        { update([this](VideoElement * e){ save_quality_scale(e); }); });
    tune.signal_value_changed.connect([this]()
        { update([this](VideoElement * e){ save_tune(e); }); });
}

void VP9_Parameters::load(VideoElement * video_element)
{
    SafeReset safe_reset(is_loading);
    CodecParametersPage::load(video_element);
    VP9_options options = video_element -> video.VP9_options;
    
    encoding_preset.set_value(options.preset);
    crf.set_value(options.crf);
    cpu_usage.set_value(options.cpu_used);
    noise_sensitivity.set_value(options.noise_sensitivity);
    quality_scale.set_value(options.quality);
    tune.set_value(options.tune_content);
}

void VP9_Parameters::save_cpu_usage(VideoElement * element)
{ element->video.VP9_options.cpu_used = cpu_usage.get_value(); }

void VP9_Parameters::save_noise_sensitivity(VideoElement * element)
{ element->video.VP9_options.cpu_used = noise_sensitivity.get_value(); }

void VP9_Parameters::save_quality_scale(VideoElement * element)
{ element->video.VP9_options.cpu_used = quality_scale.get_value(); }

void VP9_Parameters::save_tune(VideoElement * element)
{ element->video.VP9_options.cpu_used = tune.get_value(); }
