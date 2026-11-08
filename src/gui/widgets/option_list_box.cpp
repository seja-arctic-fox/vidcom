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
#include "gtkmm/listboxrow.h"

OptionListBox::OptionListBox()
{
    add_css_class("card");
    set_margin_bottom(20);
    set_selection_mode(Gtk::SelectionMode::NONE);
    set_focusable(false);
    
    signal_row_activated().connect([](Gtk::ListBoxRow * row) 
        { static_cast<OptionRow *>(row) -> on_row_activated(); });
}

OptionListBox::~OptionListBox()
{}

void OptionListBox::set_row_active(int row_index, bool active)
{
    Gtk::ListBoxRow * row = get_row_at_index(row_index);
    row -> set_activatable(active);
    row -> set_selectable(active);
}
