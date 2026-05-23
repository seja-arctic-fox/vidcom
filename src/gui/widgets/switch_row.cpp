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
