#include "../headers/gui.h"
#include <functional>

CodecParametersPage::CodecParametersPage()
:
    video_element(nullptr),
    video_queue(nullptr),
    is_loading(false),
    batch_settings(false),
    
    encoding_preset("Encoding Preset", ""),
    crf("CRF", "")
{
    encoding_preset.set_digits(0);
    encoding_preset.set_adjustment(5, 0, 16, 1);
    
    crf.set_digits(0);
    crf.set_adjustment(35, 0, 63, 1);
    
    add_css_class("flat");
    append(encoding_preset);
    append(crf);
    
    // Signály
    encoding_preset.signal_value_changed.connect([this]() 
        { update([this](VideoElement * e) { save_preset(e); }); });
    crf.signal_value_changed.connect([this]()
        { update([this](VideoElement * e) { save_crf(e); }); });
}

void CodecParametersPage::load(VideoElement * video_element)
{
    batch_settings = false;
    this -> video_element = video_element;
}

void CodecParametersPage::load_vector(std::vector<VideoElement *>& vector, VideoElement * video_element)
{
    batch_settings = true;
    this -> video_queue = &vector;
    load(video_element);
}

void CodecParametersPage::update(std::function<void(VideoElement *)> func)
{
    // Nesmí se ukládat, když se načítá
    if (is_loading)
        return;

    if (batch_settings)
        for (VideoElement * element : *video_queue)
        { func(element); }
    else
    { func(video_element); }
}

void CodecParametersPage::save_preset(VideoElement * element)
{ element -> video.AV1_options.preset = encoding_preset.get_value(); }

void CodecParametersPage::save_crf(VideoElement * element)
{ element -> video.AV1_options.crf = crf.get_value(); }
