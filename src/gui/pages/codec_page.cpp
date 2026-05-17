#include "../headers/gui.h"
#include <functional>

CodecParametersPage::CodecParametersPage()
:
    OptionListBox(),
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
    
    remove_css_class("card");
    add_css_class("flat");
    set_margin_bottom(0);
    append(encoding_preset);
    append(crf);
}

void CodecParametersPage::load(VideoElement * video_element)
{
    batch_settings = false;
    this -> video_element = video_element;
}

void CodecParametersPage::load_vector(std::vector<VideoElement *>& vector)
{
    batch_settings = true;
    this -> video_queue = &vector;
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
