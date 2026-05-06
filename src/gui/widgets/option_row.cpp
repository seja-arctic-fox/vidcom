#include "gtkmm/enums.h"
#include "pangomm/layout.h"
#include "../headers/widgets.h"

OptionRow::OptionRow(const Glib::ustring& option_title, const Glib::ustring option_caption)
:
    row_box(Gtk::Orientation::HORIZONTAL),
    text_box(Gtk::Orientation::VERTICAL),
    title(option_title, Gtk::Align::START),
    caption(option_caption, Gtk::Align::START)
{
    title.set_margin(2);
    title.set_ellipsize(Pango::EllipsizeMode::END);
    title.add_css_class("heading");
    
    caption.set_margin(2);
    caption.set_ellipsize(Pango::EllipsizeMode::END);
    caption.add_css_class("caption");
    
    text_box.set_margin(5);
    text_box.append(title);
    text_box.append(caption);
    
    row_box.set_margin(5);
    row_box.append(text_box);
    
    set_child(row_box);
}

void OptionRow::set_title(const Glib::ustring& option_title)
{ title.set_text(option_title); }

void OptionRow::set_caption(const Glib::ustring& option_caption)
{ caption.set_text(option_caption); }

void OptionRow::set_widget(Gtk::Widget& widget)
{
    widget.set_margin(10);
    widget.set_valign(Gtk::Align::CENTER);
    row_box.prepend(widget);
}
