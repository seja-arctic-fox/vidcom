#include "../headers/gui.h"
#include "src/video/video.h"

AVC_Parameters::AVC_Parameters()
:
    CodecParametersPage(),
    motion_estimation("Motion Estimation", "Enables better motion estimation. Good for better compression, but increases encoding time. "),
    adaptive_quantisation("Adaptive Quantisation", "Enables adaptive quantisation. Better quality in frames with more details. "),
    adaptive_b_frames("Adaptive B-Frames", "Enables adaptive B-frames. Inserts as much B-frames as needed for better compression. "),
    tune("Tune", "0 = film, 1 = animation, 2 = grain, 3 = stillimage")
{
    encoding_preset.set_caption("Higher value means better compression but longer encoding time. ");
    crf.set_caption("Quality level. Lower values increase quality and bitrate. ");
    
    encoding_preset.set_range(0, 9);
    crf.set_range(0, 51);
    tune.set_digits(0);
    tune.set_adjustment(0, 0, 3, 1);
    
    append(motion_estimation);
    append(adaptive_quantisation);
    append(adaptive_b_frames);
    append(tune);
    
    // Signals
    encoding_preset.signal_value_changed.connect([this]()
        { update([this](VideoElement * e) { save_preset(e); }); });
    crf.signal_value_changed.connect([this]()
        { update([this](VideoElement * e) { save_crf(e); }); });
    motion_estimation.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_motion_estimation(e); }); });
    adaptive_quantisation.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_adaptive_quantisation(e); }); });
    adaptive_b_frames.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_adaptive_b_frames(e); }); });
    tune.signal_value_changed.connect([this]()
        { update([this](VideoElement * e) { save_tune(e); }); });
}

void AVC_Parameters::load(VideoElement * video_element)
{
    SafeReset safe_reset(is_loading);
    CodecParametersPage::load(video_element);
    AVC_options options = video_element -> video.AVC_options;
    
    encoding_preset.set_value(options.preset);
    crf.set_value(options.crf);
    motion_estimation.set_state(options.motion_estimation);
    adaptive_quantisation.set_state(options.adaptive_quantisation);
    adaptive_b_frames.set_state(options.adaptive_b_frames);
    tune.set_value(options.tune);
}

void AVC_Parameters::save_preset(VideoElement * element)
{ element -> video.AVC_options.preset = encoding_preset.get_value(); }

void AVC_Parameters::save_crf(VideoElement * element)
{ element -> video.AVC_options.crf = crf.get_value(); }

void AVC_Parameters::save_motion_estimation(VideoElement * element)
{ element -> video.AVC_options.motion_estimation = motion_estimation.get_state(); }

void AVC_Parameters::save_adaptive_quantisation(VideoElement * element)
{ element -> video.AVC_options.adaptive_quantisation = adaptive_quantisation.get_state(); }

void AVC_Parameters::save_adaptive_b_frames(VideoElement * element)
{ element -> video.AVC_options.adaptive_b_frames = adaptive_b_frames.get_state(); }

void AVC_Parameters::save_tune(VideoElement * element)
{ element -> video.AVC_options.tune = tune.get_value(); }
