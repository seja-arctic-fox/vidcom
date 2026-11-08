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

SwitchRow::SwitchRow(const Glib::ustring& option_title, const Glib::ustring& option_caption)
:
    OptionRow(option_title, option_caption)
{
    switch_widget.set_can_target(false);
    set_widget(switch_widget);
}

void SwitchRow::set_state(bool state)
{ switch_widget.set_active(state); }

bool SwitchRow::get_state()
{ return switch_widget.get_active(); }

void SwitchRow::on_row_activated()
{
    set_state(!get_state());
    signal_toggled.emit();
}
