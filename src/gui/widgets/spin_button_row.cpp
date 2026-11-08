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
#include <iterator>

SpinButtonRow::SpinButtonRow(const Glib::ustring& option_title, const Glib::ustring& option_caption)
:
    OptionRow(option_title, option_caption)
{
    spin_widget.set_adjustment(adjustment);
    spin_widget.set_numeric();
    spin_widget.signal_value_changed().connect([this]() { signal_value_changed.emit(); });
    set_widget(spin_widget);
    set_selectable(false);
    set_activatable(false);
}

void SpinButtonRow::set_value(double value)
{ spin_widget.set_value(value); }

void SpinButtonRow::set_step(double step)
{ spin_widget.set_increments(step, step * 10); }

void SpinButtonRow::set_digits(int num_digits)
{ spin_widget.set_digits(num_digits); }

double SpinButtonRow::get_value()
{ return spin_widget.get_value(); }

void SpinButtonRow::set_range(double lower, double upper)
{
    spin_widget.set_range(lower, upper);
    spin_widget.set_width_chars(size(to_string(int(upper))) + spin_widget.get_digits() + 1);
}

void SpinButtonRow::set_adjustment(double value, double lower, double upper, double step)
{
    set_range(lower, upper);
    set_value(value);
    set_step(step);
}
