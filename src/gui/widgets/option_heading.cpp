#include "../headers/widgets.h"
#include "gtkmm/enums.h"
#include "pangomm/layout.h"

OptionHeading::OptionHeading(const Glib::ustring& heading_text)
:
    heading(heading_text)
{
    heading.add_css_class("title-4");
    heading.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
    description_trigger.set_icon_name("help-about-symbolic");
    description_trigger.add_css_class("flat");
    
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_margin(10);
    set_halign(Gtk::Align::CENTER);
    append(heading);
    append(description_trigger);
}

OptionHeading::~OptionHeading()
{
    if (description_popover.get_parent())
        description_popover.unparent();
}

void OptionHeading::set_popover_contents(const Glib::ustring& text)
{
    description.set_markup(text);
    description_popover.set_child(description);
    description_popover.set_parent(description_trigger);
    description_trigger.signal_clicked().connect([this](){description_popover.popup();});
}

void OptionHeading::set_heading(const Glib::ustring& heading_text)
{ heading.set_text(heading_text); }
