#include "gtkmm/enums.h"
#include "widgets.h"

CutWidget::CutWidget()
:
    box_start(Gtk::Orientation::VERTICAL),
    box_end(Gtk::Orientation::VERTICAL),
    
    separator(Gtk::Orientation::VERTICAL),
    start_label("Start time"),
    end_label("End time"), 
    
    start_time(),
    end_time()
    
{
    box_start.set_halign(Gtk::Align::CENTER);
    box_start.append(start_label);
    box_start.append(start_time);
    
    box_end.set_halign(Gtk::Align::CENTER);
    box_end.append(end_label);
    box_end.append(end_time);
    
    separator.set_margin(5);
    
    start_label.add_css_class("heading");
    end_label.add_css_class("heading");
    
    set_margin(5);
    set_halign(Gtk::Align::CENTER);
    set_orientation(Gtk::Orientation::HORIZONTAL);
    append(box_start);
    append(separator);
    append(box_end);
}

CutWidget::~CutWidget()
{}
