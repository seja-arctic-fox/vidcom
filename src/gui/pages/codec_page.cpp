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
