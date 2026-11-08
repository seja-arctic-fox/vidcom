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
#include "../headers/widgets.h"

ButtonRow::ButtonRow(const Glib::ustring& option_title, const Glib::ustring& option_caption)
:
    OptionRow(option_title, option_caption)
{
    button_widget.add_css_class("suggested-action");
    button_widget.set_hexpand(false);
    button_widget.signal_clicked().connect([this]()
        { signal_clicked.emit(); });
    set_widget(button_widget);
    set_selectable(false);
    set_activatable(false);
}

void ButtonRow::set_button_text(const Glib::ustring& text)
{ button_widget.set_label(text); }
