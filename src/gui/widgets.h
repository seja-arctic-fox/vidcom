#include "glibmm/refptr.h"
#include "gtkmm/box.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/label.h"
#include "gtkmm/adjustment.h"
#include <array>

class TimeSetter : public Gtk::Box
{
    public:
        TimeSetter();
        ~TimeSetter();
        
        void set_time(int seconds);
        void set_limit(int min_s, int max_s);
        void set_min_time(int seconds);
        void set_max_time(int seconds);
        
        int get_time();
        
    protected:
        int value;
    
        Gtk::SpinButton hours, minutes, seconds;
        Glib::RefPtr<Gtk::Adjustment> a_hours, a_minutes, a_seconds;
        Gtk::Label sep1, sep2;
        
        std::array<int, 3> compute_time(int seconds);
        int compute_seconds(std::array<int, 3> time);
};
