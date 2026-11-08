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
#include "../headers/gui.h"
#include "src/video/video.h"

HEVC_Parameters::HEVC_Parameters()
:
    CodecParametersPage(),
    motion_estimation("Motion Estimation", "Enables better motion estimation. Good for better compression, but increases encoding time. "),
    psychovisual_tuning("Psychovisual Tuning", "Enables psychovisual tuning (better for human eye). "),
    adaptive_quantisation("Adaptive Quantisation", "Enables adaptive quantisation. Better quality in frames with more details. "),
    adaptive_b_frames("Adaptive B-Frames", "Enables adaptive B-frames. Inserts as much B-frames as needed for better compression. ")
{
    encoding_preset.set_caption("Higher value means better compression but longer encoding time. ");
    crf.set_caption("Quality level. Lower values increase quality and bitrate. ");
    
    encoding_preset.set_range(0, 9);
    crf.set_range(0, 63);
    
    append(motion_estimation);
    append(psychovisual_tuning);
    append(adaptive_quantisation);
    append(adaptive_b_frames);
    
    // Signály
    encoding_preset.signal_value_changed.connect([this]() 
        { update([this](VideoElement * e) { save_preset(e); }); });
    crf.signal_value_changed.connect([this]()
        { update([this](VideoElement * e) { save_crf(e); }); });
    motion_estimation.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_motion_estimation(e); }); });
    psychovisual_tuning.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_psychovisual_tuning(e); }); });
    adaptive_quantisation.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_adaptive_quantisation(e); }); });
    adaptive_b_frames.signal_toggled.connect([this]()
        { update([this](VideoElement * e){ save_adaptive_b_frames(e); }); });
}

void HEVC_Parameters::load(VideoElement * video_element)
{
    SafeReset safe_reset(is_loading);
    CodecParametersPage::load(video_element);
    HEVC_options options = video_element -> video.HEVC_options;
    
    encoding_preset.set_value(options.preset);
    crf.set_value(options.crf);
    motion_estimation.set_state(options.motion_estimation);
    psychovisual_tuning.set_state(options.psychovisual_tuning);
    adaptive_quantisation.set_state(options.adaptive_quantisation);
    adaptive_b_frames.set_state(options.adaptive_b_frames);
}

void HEVC_Parameters::save_preset(VideoElement * element)
{ element -> video.HEVC_options.preset = encoding_preset.get_value(); }

void HEVC_Parameters::save_crf(VideoElement * element)
{ element -> video.HEVC_options.crf = crf.get_value(); }

void HEVC_Parameters::save_motion_estimation(VideoElement * element)
{ element -> video.HEVC_options.motion_estimation = motion_estimation.get_state(); }

void HEVC_Parameters::save_psychovisual_tuning(VideoElement * element)
{ element -> video.HEVC_options.psychovisual_tuning = psychovisual_tuning.get_state(); }

void HEVC_Parameters::save_adaptive_quantisation(VideoElement * element)
{ element -> video.HEVC_options.adaptive_quantisation = adaptive_quantisation.get_state(); }

void HEVC_Parameters::save_adaptive_b_frames(VideoElement * element)
{ element -> video.HEVC_options.adaptive_b_frames = adaptive_b_frames.get_state(); }
