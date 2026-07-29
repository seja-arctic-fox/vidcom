#include "gdkmm/contentprovider.h"
#include "gdkmm/drag.h"
#include "giomm/asyncresult.h"
#include "glibmm/dispatcher.h"
#include "glibmm/refptr.h"
#include "glibmm/value.h"
#include "glibmm/variant.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/dragsource.h"
#include "gtkmm/droptarget.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/frame.h"
#include "gtkmm/headerbar.h"
#include "gtkmm/image.h"
#include "gtkmm/label.h"
#include "gtkmm/listbox.h"
#include "gtkmm/listboxrow.h"
#include "gtkmm/progressbar.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/togglebutton.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include <functional>
#include <gtkmm.h>
#include <adwaita.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include "../../video/video.h"
#include "sigc++/connection.h"
#include "sigc++/signal.h"
#include "widgets.h"

#ifndef GUI
#define GUI

// Bezpečný reset stavu načítání
// Bez tohoto by mohla nastat výjimka a pak už by všechna nastavení nešla změnit, protože by to blokoval is_loading
// SafeReset po skončení funkce zavolá vždy destruktor, který to nastaví zpět, i když se ta dotyčná metoda nedokončí
struct SafeReset
{
    bool &value;
    SafeReset(bool &v) : value(v) { value = true; }
    ~SafeReset() { value = false; }
};

// Stav kódování
struct EncodingProgress
{
    std::string video_name;
    int progress_percent;
    float current_time;
    int current_index;
    int total_count;
};

// Výsledek kódování
struct EncodingResult
{
    fs::path video_path;
    int exit_status;
    bool was_cancelled;
};

// Prvek ve frontě kódování
class VideoElement : public Gtk::Frame
{
    public:
        VideoElement(std::string input_path);
        ~VideoElement();
        void update_labels();

        Video video;
        VideoInfo video_info;

        sigc::signal<void(VideoElement *)> signal_remove;

        protected:

            // Popisky vlastností videa
            Gtk::Image drag_handle_icon, video_thumbnail;
            Gtk::Frame video_thumbnail_frame;
            Gtk::Label video_name_text, resolution_text, framerate_text, duration_text, mode_text, size_text;
            RoundedImage css_rounded;
            
            // Tlačítko pro odstranění prvku z fronty
            Gtk::Button remove_element_button;
            void on_click_remove_element();

            // Boxy pro rozložení
            Gtk::Box main_hbox;
            Gtk::Box label_vbox;

            void on_remove_clicked();

            // Změna pořadí prvků ve frontě
            Glib::RefPtr<Gtk::DragSource> drag_source;
            Glib::RefPtr<Gtk::DropTarget> drop_target;

            Glib::RefPtr<Gdk::ContentProvider> on_drag_prepare(double, double);
            void on_drag_begin(const Glib::RefPtr<Gdk::Drag>& drag);
            bool on_drop(const Glib::ValueBase& value, double, double);
};

class RunnerPanel : public Gtk::HeaderBar
{
    public:
        RunnerPanel();
        ~RunnerPanel();

        // Aktualizace informací o postupu
        void update_encoding_progress(const EncodingProgress& progress);
        void set_encoding_state(bool is_encoding);
        void update_status(const std::string& status, const std::string& css_class = "");
        void block_encoding_button(bool block = true);
        void set_loading_state(bool is_loading);
        void request_encoding_button_unblock(){ request_button_unblock = true; };
        void update_loading_progress(int video_index, int video_count);
        void show_queue_button(bool show);
        void set_title(VideoElement * video_element);
        void set_title_multiple(std::vector<VideoElement*>);
        void clear_title();
        
        // Signály
        sigc::signal<void()> signal_start_encoding;
        sigc::signal<void()> signal_stop_encoding;
        sigc::signal<void()> signal_toggle_queue;

    protected:
        bool isEncoding = false;
        bool request_button_unblock = false;

        Gtk::Button queue_display_button;
        Gtk::ProgressBar EncodingProgressBar;
        Gtk::Button EncodingButton;
        Gtk::Label WindowTitle;
        Gtk::Image EncodingIconStatus;
        Gtk::Label EncodingTextStatus;

        void on_start_stop_clicked();
};

// Interaktivní fronta kódování, do které bude možné vkládat videa
class QueueFrame : public Gtk::Box
{
    public:
        QueueFrame();
        ~QueueFrame();

        void add_video(const std::string& input_path);
        std::vector<Video *> get_all_videos();
        sigc::signal<void(VideoElement *)> signal_video_selected;
        sigc::signal<void(std::vector<VideoElement*>)> signal_all_videos_selected;
        sigc::signal<void()> signal_nothing_selected;
        sigc::signal<void(bool)> signal_loading_videos;
        sigc::signal<void(int, int)> signal_loading_videos_count;
        sigc::signal<void()> signal_queue_cleared;
        sigc::signal<void()> signal_enable_encoding;
    
    protected:
        // Prostor pro prvky fronty a seznam prvků
        Gtk::ScrolledWindow scrolled_window;
        Gtk::ListBox video_listbox;

        // Prázdná fronta
        AdwStatusPage * queue_empty_status;

        // Spodní lišta
        Gtk::Box footer_box;
        Gtk::Button import_video_button;

        // Tlačítko na vymazání celé fronty a horní lišta
        Gtk::Box header_box;
        Gtk::Box clear_queue_box;
        Gtk::Box select_all_box;
        Gtk::Image clear_queue_icon;
        Gtk::Image select_all_icon;
        Gtk::Label clear_queue_text;
        Gtk::Label select_all_text;
        Gtk::Button clear_queue_button;
        Gtk::ToggleButton select_all_button;

        // Drag and drop
        Glib::RefPtr<Gtk::DropTarget> drag_and_drop_target;
        sigc::connection idle_handler;

        // Metody
        void on_clear_clicked();
        void on_select_all_clicked();
        bool on_drop(const Glib::ValueBase& value, double, double);
        void error_toast_not_a_video(string file);
        void on_row_selected(Gtk::ListBoxRow * row);
};

// Obecná stránka pro parametry
class CodecParametersPage : public OptionListBox
{
    public:
        CodecParametersPage();
        virtual ~CodecParametersPage() = default;
        
        virtual void load(VideoElement * video_element);
        virtual void load_vector(std::vector<VideoElement *>& vector);
        
    protected:
        VideoElement * video_element;
        std::vector<VideoElement *> * video_queue;
        bool is_loading;
        bool batch_settings;
        
        SpinButtonRow encoding_preset;
        SpinButtonRow crf;
        
        virtual void update(std::function<void(VideoElement *)> func);
        void save_preset(VideoElement * element);
        void save_crf(VideoElement * element);
};

// Stránka parametrů pro AV1
class AV1_Parameters : public CodecParametersPage
{
    public: 
        AV1_Parameters();
        void load(VideoElement * video_element) override;
        
    protected:
        SwitchRow film_grain_synthesis;
        SpinButtonRow film_grain_level;
        SwitchRow better_details;
        SwitchRow psychovisual_tuning;
        SwitchRow variance_boost;
        
        void save_preset(VideoElement * element);
        void save_crf(VideoElement * element);
        void save_film_grain(VideoElement * element);
        void save_better_details(VideoElement * element);
        void save_psychovisual_tuning(VideoElement * element);
        void save_variance_boost(VideoElement * element);
        
};

// Stránka parametrů pro HEVC
class HEVC_Parameters : public CodecParametersPage
{
    public: 
        HEVC_Parameters();
        void load(VideoElement * video_element) override;

    protected:
        SwitchRow motion_estimation;
        SwitchRow psychovisual_tuning;
        SwitchRow adaptive_quantisation;
        SwitchRow adaptive_b_frames;
        
        void save_preset(VideoElement * element);
        void save_crf(VideoElement * element);
        void save_motion_estimation(VideoElement * element);
        void save_psychovisual_tuning(VideoElement * element);
        void save_adaptive_quantisation(VideoElement * element);
        void save_adaptive_b_frames(VideoElement * element);

};

// Stránka parametrů pro VP9
class VP9_Parameters : public CodecParametersPage
{
    public: 
        VP9_Parameters();
        void load(VideoElement * video_element);
        
    protected:
        SpinButtonRow cpu_usage;
        SpinButtonRow noise_sensitivity;
        SpinButtonRow quality_scale;
        SpinButtonRow tune;
        
        void save_preset(VideoElement * element);
        void save_crf(VideoElement * element);
        void save_cpu_usage(VideoElement * element);
        void save_noise_sensitivity(VideoElement * element);
        void save_quality_scale(VideoElement * element);
        void save_tune(VideoElement * element);
};

// Stránka pro označené video ve frontě. Obsahuje základní nastavení pro každé video individuálně
class SettingsPage : public Gtk::ScrolledWindow
{
    public:
        SettingsPage();
        ~SettingsPage();

        // Aktualizace nastavení videa
        void read_video_options(VideoElement * video);
        void read_video_vector_options(std::vector<VideoElement *> video_vector);

    protected:
        // Nastavení lze provádět pro jedno nebo více označených videí
        VideoElement * video_element;
        std::vector<VideoElement *> video_queue;
        bool batch_settings;
        bool is_loading;
        string output_path;
        Gtk::Box window_content;

        // Režim kódování
        OptionHeading mode_heading;
        OptionListBox mode_listbox;
        RadioButtonRow compress_row;
        RadioButtonRow archive_row;
        
        // Pro kompresi: 
        SpinButtonRow target_size_row;
        SpinButtonRow res_row, fps_row;
        
        // Střih
        OptionHeading cut_heading;
        OptionListBox cut_listbox;
        SwitchRow cut_switch;
        CutWidget cut_widget;

        // Výstupní složka a prefix
        OptionHeading output_heading;
        ButtonRow output_row;
        FieldRow prefix_row;
        OptionListBox output_listbox;
        
        // Kodeky
        OptionHeading codec_heading;
        OptionListBox codec_listbox;
        AdwViewStack * codec_pages;
        AdwInlineViewSwitcher * codec_switch;
        AV1_Parameters av1_page;
        HEVC_Parameters hevc_page;
        VP9_Parameters vp9_page;

        // Ukládací funkce
        void save_archive_mode(VideoElement * element);
        void save_compress_mode(VideoElement * element);
        void save_target_size(VideoElement * element);
        void save_target_res(VideoElement * element);
        void save_target_fps(VideoElement * element);
        void save_cut(VideoElement * element);
        void save_prefix(VideoElement * element);
        void save_output_path(VideoElement * element);
        void save_codec(VideoElement * element);
        
        void load_options_into_GUI(Video * video);
        void update(void (SettingsPage::*func)(VideoElement *));
        void set_output_path();
        void on_folder_selected(Glib::RefPtr<Gio::AsyncResult> &result, 
                                Glib::RefPtr<Gtk::FileDialog> folder_picker);
};


class ResultsPage : public Gtk::Box
{
    public:
        ResultsPage();
        ~ResultsPage();
    
        void load_results(std::vector<EncodingResult> encoding_results);
        sigc::signal<void()> signal_close_results;
        
    protected:
        Gtk::Label result_label;
        Gtk::ScrolledWindow scrolled_window;
        Gtk::ListBox results_listbox;
        Gtk::Button ok_button;
};

class ResultRow : public Gtk::Box
{
    friend class ResultsPage;
    
    public:
        ResultRow(fs::path video_path, int status);
        ~ResultRow();
        
    protected:
        fs::path video_path;
        int status;
    
        Gtk::Label result_label, video_name, status_text;
        Gtk::Image status_icon;
        Gtk::Button output_folder_button;
        Gtk::Box box_right;
        
        void open_video();
        void on_video_folder_open();
        void set_status();
        void set_output_folder_button();
};

class MainWindow : public Gtk::Window
{
    public:
        MainWindow();
        ~MainWindow();
        
        void show_toast(char const * message);

    protected:
        RunnerPanel runner_panel;
        QueueFrame video_queue;
        SettingsPage options_page;
        Gtk::Stack main_page_stack;
        AdwStatusPage * queue_empty_page;
        AdwStatusPage * encoding_page;
        Gtk::ProgressBar encoding_page_progress;
        Gtk::Button add_videos_pill_button;
        ResultsPage results_page;
        
        // Layout aplikace
        Glib::RefPtr<Gio::Menu> main_menu;
        Gtk::MenuButton menu_button;
        Gtk::Button add_videos_button;
        AdwHeaderBar * sidebar_header;
        AdwToolbarView * sidebar_view;
        AdwToolbarView * content_view;
        AdwToastOverlay * toast_overlay;
        AdwOverlaySplitView * split_view;
        
        void on_window_resize(int width, int height);
        void on_import_video_clicked();
        void display_about_dialog(const Glib::VariantBase&);
        void display_preferences(const Glib::VariantBase&);
        void file_picker_add_videos(const Glib::RefPtr<Gio::AsyncResult>& result, Glib::RefPtr<Gtk::FileDialog> file_picker);
        
        // Vlákno pro kódování videí, synchronizace
        std::thread encoding_thread;
        std::atomic<bool> is_encoding;
        std::mutex encoding_mutex;

        // Dispatcher vláken
        Glib::Dispatcher progress_dispatcher;
        Glib::Dispatcher completion_dispatcher;

        EncodingProgress current_progress;
        std::vector<EncodingResult> encoding_results;
        bool queue_lock = false;

        // Kódování
        void start_encoding();
        void stop_encoding();
        void encoding_worker();
        void on_progress_update();
        void on_encoding_complete();
        void show_results_dialog();
};

#endif
