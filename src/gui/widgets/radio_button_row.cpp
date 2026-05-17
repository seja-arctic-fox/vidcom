#include "../headers/widgets.h"

RadioButtonRow::RadioButtonRow(const Glib::ustring& option_title, const Glib::ustring& option_caption)
:
    OptionRow(option_title, option_caption)
{ set_widget(radio_widget); }

void RadioButtonRow::set_group(RadioButtonRow& row)
{ radio_widget.set_group(row.radio_widget); }

void RadioButtonRow::set_state(bool state)
{ radio_widget.set_active(state); }

bool RadioButtonRow::get_state()
{ return radio_widget.get_active(); }

void RadioButtonRow::on_row_activated()
{
    radio_widget.set_active();
    signal_toggled.emit();
}
