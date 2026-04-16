#include "glibmm/refptr.h"
#include "gtkmm/box.h"
#include "gtkmm/flowbox.h"
#include "gtkmm/separator.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/label.h"
#include "gtkmm/adjustment.h"
#include <array>

class TimeSetter : public Gtk::Box
{
    public:
        TimeSetter();
        ~TimeSetter();
        
        void set_seconds(int seconds);
        void set_range(int min_s, int max_s);
        void set_min_time(int seconds);
        void set_max_time(int seconds);
        
        int get_seconds();
        std::array<int, 3> get_time();
        int get_min();
        int get_max();
        
    protected:
        Gtk::SpinButton hours, minutes, seconds;
        Glib::RefPtr<Gtk::Adjustment> a_hours, a_minutes, a_seconds;
        Gtk::Label sep1, sep2;
        
        int min_s, max_s;
        
        std::array<int, 3> compute_time(int seconds);
        int compute_seconds(std::array<int, 3> time);
        void update_adjustments();
        void resolve_overflow(Gtk::SpinButton * widget);
};

class CutWidget : public Gtk::FlowBox
{
    public:
        CutWidget();
        ~CutWidget();
    
    protected:
        Gtk::Box box_start, box_end;
        Gtk::Separator separator;
        Gtk::Label start_label, end_label;
        TimeSetter start_time, end_time;
};
