#include "gtkmm/enums.h"
#include "widgets.h"

CutWidget::CutWidget()
:
    box_start(Gtk::Orientation::VERTICAL),
    box_end(Gtk::Orientation::VERTICAL),
    start_label("Start time"),
    end_label("End time"), 
    start_time(),
    end_time(), 
    
    css()
    
{
    box_start.set_halign(Gtk::Align::CENTER);
    box_start.append(start_label);
    box_start.append(start_time);
    box_start.set_margin(5);
    
    box_end.set_halign(Gtk::Align::CENTER);
    box_end.append(end_label);
    box_end.append(end_time);
    box_end.set_margin(5);
    
    start_label.add_css_class("heading");
    end_label.add_css_class("heading");
    
    add_css_class("no_highlight");
    set_halign(Gtk::Align::CENTER);
    set_orientation(Gtk::Orientation::HORIZONTAL);
    append(box_start);
    append(box_end);
    get_child_at_index(0) -> set_focusable(false);
    get_child_at_index(1) -> set_focusable(false);
}

CutWidget::~CutWidget()
{}
