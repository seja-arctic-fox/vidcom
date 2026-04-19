#include "glibmm/refptr.h"
#include "gtkmm/box.h"
#include "gtkmm/cssprovider.h"
#include "gtkmm/flowbox.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/label.h"
#include "gtkmm/adjustment.h"
#include "sigc++/signal.h"
#include "src/video/video.h"
#include <array>

class NoFlowBoxHL
{
    public:
        NoFlowBoxHL()
        {
            css = Gtk::CssProvider::create();
            css -> load_from_data(
                ".no_highlight flowboxchild {background: none; }"
            );
            
            Gtk::StyleContext::add_provider_for_display(
                Gdk::Display::get_default(), 
                css, 
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        
        ~NoFlowBoxHL(){};
        
    private:
        inline static Glib::RefPtr<Gtk::CssProvider> css;
};

class TimeSetter : public Gtk::Box
{
    friend class CutWidget;
    
    public:
        TimeSetter();
        ~TimeSetter();
        
        void set_seconds(int seconds);
        void set_range(int min_s, int max_s);
        void set_min(int seconds = 0);
        void set_max(int seconds = INT_MAX);
        
        int get_seconds();
        std::array<int, 3> get_time();
        int get_min();
        int get_max();
        
    protected:
        Gtk::SpinButton hours, minutes, seconds;
        Glib::RefPtr<Gtk::Adjustment> a_hours, a_minutes, a_seconds;
        Gtk::Label sep1, sep2;
        
        int min_s, max_s;
        
        sigc::signal<void(TimeSetter *)> signal_cut_change;
        std::array<int, 3> compute_time(int seconds);
        int compute_seconds(std::array<int, 3> time);
        void update_adjustments();
        void resolve_overflow(Gtk::SpinButton * widget);
};

class CutWidget : public Gtk::FlowBox
{
    friend class TimeSetter;
    
    public:
        CutWidget();
        ~CutWidget();
    
        void set_cut(int video_duration_s, Cut cut_info);
        int get_start();
        int get_end();
        
        sigc::signal<void()> signal_cut_change;
        
    protected:
        Gtk::Box box_start, box_end;
        Gtk::Label start_label, end_label;
        TimeSetter start_time, end_time;

        NoFlowBoxHL css;
        bool updating = false;
        
        void update_limits(TimeSetter * setter = nullptr);
};
