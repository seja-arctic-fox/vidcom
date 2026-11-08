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
#include "gtkmm/enums.h"
#include "pangomm/layout.h"

OptionHeading::OptionHeading(const Glib::ustring& heading_text)
:
    heading(heading_text)
{
    heading.add_css_class("title-4");
    heading.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
    description_trigger.set_icon_name("help-about-symbolic");
    description_trigger.add_css_class("flat");
    
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_margin(10);
    set_halign(Gtk::Align::CENTER);
    append(heading);
    append(description_trigger);
}

OptionHeading::~OptionHeading()
{
    if (description_popover.get_parent())
        description_popover.unparent();
}

void OptionHeading::set_popover_contents(const Glib::ustring& text)
{
    description.set_markup(text);
    description_popover.set_child(description);
    description_popover.set_parent(description_trigger);
    description_trigger.signal_clicked().connect([this](){description_popover.popup();});
}

void OptionHeading::set_heading(const Glib::ustring& heading_text)
{ heading.set_text(heading_text); }
