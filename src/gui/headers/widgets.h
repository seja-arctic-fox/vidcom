#include "glibmm/refptr.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/cssprovider.h"
#include "gtkmm/flowbox.h"
#include "gtkmm/listbox.h"
#include "gtkmm/listboxrow.h"
#include "gtkmm/popover.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/label.h"
#include "gtkmm/adjustment.h"
#include "gtkmm/switch.h"
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

class RoundedImage
{
    public:
        RoundedImage()
        {
            auto css = Gtk::CssProvider::create();
            css -> load_from_data(
                ".rounded {border-radius: 8px;}"
            );
            
            Gtk::StyleContext::add_provider_for_display(
                Gdk::Display::get_default(), 
                css, 
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        
        ~RoundedImage(){};
        
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

class OptionHeading : public Gtk::Box
{
    public:
        OptionHeading(const Glib::ustring& heading_text);
        ~OptionHeading();
        
        void set_popover_contents(const Glib::ustring& text);
        void set_heading(const Glib::ustring& heading_text);
        
    protected:
        Gtk::Label heading, description;
        Gtk::Button description_trigger;
        Gtk::Popover description_popover;
};

class OptionListBox : public Gtk::ListBox
{
    public:
        OptionListBox();
        ~OptionListBox();
        
        void set_row_active(int row_index, bool active);
};

class OptionRow : public Gtk::ListBoxRow
{
    public:
        OptionRow(const Glib::ustring& option_title = "", const Glib::ustring& option_caption = "");
        virtual ~OptionRow() = default;
        
        void set_title(const Glib::ustring& option_title);
        void set_caption(const Glib::ustring& option_caption);
        virtual void on_row_activated() {};
        
    protected:
        void set_widget(Gtk::Widget& widget);
        
        Gtk::Box row_box, text_box;
        Gtk::Label title, caption;
};

class SwitchRow : public OptionRow
{
    public:
        SwitchRow(const Glib::ustring& option_title = "", const Glib::ustring& option_caption = "");
        void set_state(bool state);
        bool get_state();
        void on_row_activated();
        
        sigc::signal<void()> signal_toggled;
    
    protected:
        Gtk::Switch switch_widget;
};

class ButtonRow : public OptionRow
{
    public:
        ButtonRow(const Glib::ustring& option_title = "", const Glib::ustring& option_caption = "");
        void set_button_text(const Glib::ustring& text);
        
        sigc::signal<void()> signal_clicked;
    
    protected:
        Gtk::Button button_widget;
};

class RadioButtonRow : public OptionRow
{
    public:
        RadioButtonRow(const Glib::ustring& option_title = "", const Glib::ustring& option_caption = "");
        void set_group(RadioButtonRow& row);
        void on_row_activated();
        void set_state(bool state);
        bool get_state();
        
        sigc::signal<void()> signal_toggled;
    
    protected:
        Gtk::CheckButton radio_widget;
};

class SpinButtonRow : public OptionRow
{
    public:
        SpinButtonRow(const Glib::ustring& option_title = "", const Glib::ustring& option_caption = "");
        void set_adjustment(double value, double lower, double upper, double step);
        void set_value(double value);
        void set_range(double lower, double upper);
        void set_digits(int num_digits);
        void set_step(double step);
        double get_value();
        
        sigc::signal<void()> signal_value_changed;
        
    protected:
        Gtk::SpinButton spin_widget;
        Glib::RefPtr<Gtk::Adjustment> adjustment;
};
