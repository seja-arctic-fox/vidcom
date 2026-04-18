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
    
    start_time.signal_cut_change.connect(sigc::mem_fun(*this, &CutWidget::update_limits));
    end_time.signal_cut_change.connect(sigc::mem_fun(*this, &CutWidget::update_limits));
    
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

void CutWidget::set_cut(int video_duration_s, Cut cut_info)
{
    updating = true;
    
    this -> start_time.set_min(0);
    this -> end_time.set_min(0);
    this -> start_time.set_max(video_duration_s);
    this -> end_time.set_max(video_duration_s);
    
    this -> start_time.set_seconds(cut_info.startTime);
    this -> end_time.set_seconds(cut_info.endTime);
    
    updating = false;
    update_limits();
}

int CutWidget::get_start()
{
    return start_time.get_seconds();
}

int CutWidget::get_end()
{
    return end_time.get_seconds();
}

void CutWidget::update_limits(TimeSetter * setter)
{
    if (this -> updating) return;
    updating = true;
    
    this -> start_time.set_max(end_time.get_seconds() - 1);
    this -> end_time.set_min(start_time.get_seconds() + 1);
    
    if (setter)
    {
        if (setter == &start_time) end_time.update_adjustments();
        if (setter == &end_time) start_time.update_adjustments();
    }
    
    signal_cut_change.emit();
    updating = false;
}