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

FieldRow::FieldRow(const Glib::ustring& option_title, const Glib::ustring& option_caption)
:
    OptionRow(option_title, option_caption)
{
    entry_widget.signal_changed().connect([this](){ signal_changed.emit(); });
    set_widget(entry_widget);
    set_selectable(false);
    set_activatable(false);
}

void FieldRow::set_field_width(int width)
{ entry_widget.set_width_chars(width); }

void FieldRow::set_field_text(const Glib::ustring& text)
{ entry_widget.set_text(text); }

void FieldRow::set_field_placeholder_text(const Glib::ustring& text)
{ entry_widget.set_placeholder_text(text); }

Glib::ustring FieldRow::get_field_text()
{ return entry_widget.get_text(); }
