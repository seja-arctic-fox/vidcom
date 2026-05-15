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
    add_css_class("flat");
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
