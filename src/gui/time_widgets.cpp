#include "gtkmm/enums.h"
#include "gtkmm/spinbutton.h"
#include "widgets.h"
#include <vector>

TimeSetter::TimeSetter()
:
    a_hours     (Gtk::Adjustment::create(0, 0, 99)),
    a_minutes   (Gtk::Adjustment::create(0, 0, 59)),
    a_seconds   (Gtk::Adjustment::create(0, 0, 59)),
    
    sep1(":"),
    sep2(":")
{
    hours.set_adjustment(a_hours);
    minutes.set_adjustment(a_minutes);
    seconds.set_adjustment(a_seconds);
    
    std::vector<Gtk::SpinButton *> sb_v = {&hours, &minutes, &seconds};
    for (Gtk::SpinButton * sb : sb_v)
    {
        sb -> set_orientation(Gtk::Orientation::VERTICAL);
        sb -> set_numeric();
        sb -> set_width_chars(2);
        sb -> set_size_request(50);
        sb -> add_css_class("title-2");
        sb -> set_margin(5);
    }
    
    sep1.add_css_class("title-2");
    sep2.add_css_class("title-2");
    
    set_margin(10);
    set_halign(Gtk::Align::CENTER);
    set_orientation(Gtk::Orientation::HORIZONTAL);
    
    append(hours);
    append(sep1);
    append(minutes);
    append(sep2);
    append(seconds);
}

TimeSetter::~TimeSetter()
{}
