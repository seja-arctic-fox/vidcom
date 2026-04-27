#include "gtkmm/enums.h"
#include "gtkmm/spinbutton.h"
#include "widgets.h"
#include <climits>
#include <vector>

TimeSetter::TimeSetter()
:
    a_hours     (Gtk::Adjustment::create(0, 0, 99)),
    a_minutes   (Gtk::Adjustment::create(0, 0, 59)),
    a_seconds   (Gtk::Adjustment::create(0, 0, 59)),
    
    sep1(" : "),
    sep2(" : "), 
    
    min_s(0),
    max_s(INT_MAX)
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
        sb -> signal_value_changed().connect(sigc::mem_fun(*this, &TimeSetter::update_adjustments));
        sb -> signal_value_changed().connect(sigc::bind(sigc::mem_fun(*this, &TimeSetter::resolve_overflow), sb));
        sb -> signal_value_changed().connect([this]() {signal_cut_change.emit(this);});
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

void TimeSetter::set_seconds(int seconds)
{
    std::array<int, 3> new_time = compute_time(seconds);
    
    // Nastavit čas do polí
    if (this -> min_s <= seconds && seconds <= this -> max_s)
    {
        this -> hours.set_value(new_time[0]);
        this -> minutes.set_value(new_time[1]);
        this -> seconds.set_value(new_time[2]);
    }
    
    // Aktualizovat limity
    update_adjustments();
}

void TimeSetter::update_adjustments()
{
    auto current_time = get_time();
    auto min_time = compute_time(this -> min_s);
    auto max_time = compute_time(this -> max_s);
    
    int min_time_h = min_time[0];
    int min_time_m = current_time[0] > min_time[0] ? 0 : min_time[1];
    int min_time_s = current_time[0] > min_time[0] || current_time[1] > min_time[1] ? 0 : min_time[2];
    
    int max_time_h = max_time[0];
    int max_time_m = current_time[0] < max_time[0] ? 59 : max_time[1];
    int max_time_s = current_time[0] < max_time[0] || current_time[1] < max_time[1] ? 59 : max_time[2];
    
    a_hours -> set_lower(min_time_h);
    a_minutes -> set_lower(min_time_m);
    a_seconds -> set_lower(min_time_s);
    
    a_hours -> set_upper(max_time_h);
    a_minutes -> set_upper(max_time_m);
    a_seconds -> set_upper(max_time_s);
}

void TimeSetter::resolve_overflow(Gtk::SpinButton * widget)
{
    std::vector<Gtk::SpinButton * > sb_v = {&hours, &minutes, &seconds};
    std::array<int, 3> min_time = compute_time(min_s);
    std::array<int, 3> max_time = compute_time(max_s);
    std::array<int, 3> * limit_ptr = nullptr;
    
    if (get_seconds() > this -> max_s) { limit_ptr = &max_time; }
    else if (get_seconds() < this -> min_s) { limit_ptr = &min_time; }
    else { return; }
    
    unsigned short i = 0;
    for (Gtk::SpinButton * sb : sb_v)
    {
        if (sb == widget) { i++; continue; }
        
        sb -> set_value((*limit_ptr)[i]);
        i++;
    }
}

void TimeSetter::set_min(int seconds)
{
    this -> min_s = seconds;
    update_adjustments();
}

void TimeSetter::set_max(int seconds)
{
    this -> max_s = seconds;
    update_adjustments();
}

void TimeSetter::set_range(int min_s, int max_s)
{
    set_min(min_s);
    set_max(max_s);
}

std::array<int, 3> TimeSetter::compute_time(int seconds)
{
    std::array<int, 3> time;
    
    int hours = (seconds / 3600);
    int minutes = ((seconds / 60) - (hours * 60));
    seconds -= (hours * 3600) + (minutes * 60);
    
    time = {hours, minutes, seconds};
    return time;
}

int TimeSetter::compute_seconds(std::array<int, 3> time)
{
    return time[2] + time[1] * 60 + time[0] * 3600;
}

int TimeSetter::get_seconds()
{
    return compute_seconds(get_time());
}

std::array<int, 3> TimeSetter::get_time()
{
    return {
            int(hours.get_value()), 
            int(minutes.get_value()), 
            int(seconds.get_value())
            };
}

int TimeSetter::get_min()
{
    return this -> min_s;
}
int TimeSetter::get_max()
{
    return this -> max_s;
}
