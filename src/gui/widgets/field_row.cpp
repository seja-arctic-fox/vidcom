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
