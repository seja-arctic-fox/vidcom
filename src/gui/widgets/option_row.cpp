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
#include "gtkmm/enums.h"
#include "pangomm/layout.h"
#include "../headers/widgets.h"

OptionRow::OptionRow(const Glib::ustring& option_title, const Glib::ustring& option_caption)
:
    row_box(Gtk::Orientation::HORIZONTAL),
    text_box(Gtk::Orientation::VERTICAL),
    title(option_title, Gtk::Align::START),
    caption(option_caption, Gtk::Align::START)
{
    title.set_margin(2);
    title.set_ellipsize(Pango::EllipsizeMode::END);
    title.add_css_class("heading");
    
    caption.set_margin(2);
    caption.set_ellipsize(Pango::EllipsizeMode::END);
    caption.add_css_class("caption");
    
    text_box.set_margin(5);
    text_box.append(title);
    text_box.append(caption);
    
    row_box.set_margin(5);
    row_box.append(text_box);
    
    set_child(row_box);
}

void OptionRow::set_title(const Glib::ustring& option_title)
{ title.set_text(option_title); }

void OptionRow::set_caption(const Glib::ustring& option_caption)
{ caption.set_text(option_caption); }

void OptionRow::set_widget(Gtk::Widget& widget)
{
    widget.set_margin(10);
    widget.set_valign(Gtk::Align::CENTER);
    widget.set_focusable(false);
    row_box.prepend(widget);
}
