#include "../headers/widgets.h"
#include "gtkmm/listboxrow.h"

OptionListBox::OptionListBox()
{
    add_css_class("navigation-sidebar");
    add_css_class("card");
    set_margin_bottom(20);
    
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
