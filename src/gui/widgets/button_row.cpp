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
