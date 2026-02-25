#include "gdkmm/contentprovider.h"
#include "gdkmm/drag.h"
#include "giomm/asyncresult.h"
#include "glibmm/dispatcher.h"
#include "glibmm/refptr.h"
#include "glibmm/value.h"
#include "gtkmm/adjustment.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/dialog.h"
#include "gtkmm/dragsource.h"
#include "gtkmm/droptarget.h"
#include "gtkmm/entry.h"
#include "gtkmm/enums.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/flowbox.h"
#include "gtkmm/flowboxchild.h"
#include "gtkmm/frame.h"
#include "gtkmm/headerbar.h"
#include "gtkmm/image.h"
#include "gtkmm/label.h"
#include "gtkmm/listbox.h"
#include "gtkmm/listboxrow.h"
#include "gtkmm/paned.h"
#include "gtkmm/popover.h"
#include "gtkmm/progressbar.h"
#include "gtkmm/scale.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/switch.h"
#include "gtkmm/togglebutton.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include <gtkmm.h>
#include <adwaita.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include "../video/video.h"
#include "sigc++/signal.h"

#ifndef GUI
#define GUI

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
    std::string video_name;
    std::string video_path;
    int exit_status;
    bool was_cancelled;
};

class RunnerPanel : public Gtk::Box
{
    public:
        RunnerPanel();
        ~RunnerPanel();

        // Aktualizace informací o postupu
        void update_progress(const EncodingProgress& progress);
        void set_encoding_state(bool is_encoding);
        void update_status(const std::string& status, const std::string& css_class = "");
        void block_encoding_button(bool block);
        
        // Signály
        sigc::signal<void()> signal_start_encoding;
        sigc::signal<void()> signal_stop_encoding;

    protected:
        bool isEncoding;

        Gtk::ProgressBar EncodingProgressBar;
        Gtk::Button EncodingButton;
        Gtk::Label WindowTitle;
        Gtk::Image EncodingIconStatus;
        Gtk::Label EncodingTextStatus;

        void on_start_stop_clicked();
};

// Prvek ve frontě kódování
class VideoElement : public Gtk::Frame
{
    public:
        VideoElement(std::string input_path);
        ~VideoElement();
        void update_labels();

        Video video;

        sigc::signal<void(VideoElement *)> signal_remove;

        protected:
            VideoInfo video_info;

            // Popisky vlastností videa
            Gtk::Image drag_handle_icon;
            Gtk::Image video_thumbnail;
            Gtk::Label video_name_text;
            Gtk::Label resolution_text;
            Gtk::Label framerate_text;
            Gtk::Label duration_text;
            Gtk::Label mode_text;
            
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
    
    protected:
        // Prostor pro prvky fronty a seznam prvků
        Gtk::ScrolledWindow scrolled_window;
        Gtk::ListBox video_listbox;

        // Prázdná fronta
        Gtk::Box empty_queue_box;
        Gtk::Image empty_queue_icon;
        Gtk::Label empty_queue_label;
        Gtk::Label empty_queue_caption;

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

        // Metody
        void on_clear_clicked();
        void on_select_all_clicked();
        void on_import_video_clicked();
        bool on_drop(const Glib::ValueBase& value, double, double);
        void error_dialog_not_a_video();
        void on_row_selected(Gtk::ListBoxRow * row);
        void file_picker_add_videos(const Glib::RefPtr<Gio::AsyncResult>& result, Glib::RefPtr<Gtk::FileDialog> file_picker);
};

// Stránka parametrů pro AV1
class AV1_Parameters : public Gtk::ListBox
{
    friend class VideoSettings_VBox;

    public: 
        AV1_Parameters(VideoElement * video_element);
        ~AV1_Parameters();

    protected:
        VideoElement * video_element;
        bool is_loading;

        Gtk::Label preset_text, crf_text, fgs_text, fgl_text, bd_text, pt_text, vb_text;
        Gtk::Label preset_caption, crf_caption, fgs_caption, fgl_caption, bd_caption, pt_caption, vb_caption;
        Gtk::Box preset_hbox, crf_hbox, fgs_hbox, fgl_hbox, bd_hbox, pt_hbox, vb_hbox;
        Gtk::Box preset_vbox, crf_vbox, fgs_vbox, fgl_vbox, bd_vbox, pt_vbox, vb_vbox;
        Gtk::SpinButton preset_w, crf_w, fgl_w;
        Gtk::Switch fgs_w, bd_w, pt_w, vb_w;

        void load();
        void update();
        void on_select_row(Gtk::ListBoxRow * row);
};

// Stránka parametrů pro HEVC
class HEVC_Parameters : public Gtk::ListBox
{
    friend class VideoSettings_VBox;

    public: 
        HEVC_Parameters(VideoElement * video_element);
        ~HEVC_Parameters();

    protected:
        VideoElement * video_element;
        bool is_loading;

        Gtk::Label preset_text, crf_text, me_text, aq_text, pt_text, ab_text;
        Gtk::Label preset_caption, crf_caption, me_caption, aq_caption, pt_caption, ab_caption;
        Gtk::Box preset_hbox, crf_hbox, me_hbox, aq_hbox, pt_hbox, ab_hbox;
        Gtk::Box preset_vbox, crf_vbox, me_vbox, aq_vbox, pt_vbox, ab_vbox;
        Gtk::SpinButton preset_w, crf_w;
        Gtk::Switch me_w, aq_w, pt_w, ab_w;

        void load();
        void update();
        void on_select_row(Gtk::ListBoxRow * row);
};

// Stránka parametrů pro VP9
class VP9_Parameters : public Gtk::ListBox
{
    friend class VideoSettings_VBox;

    public: 
        VP9_Parameters(VideoElement * video_element);
        ~VP9_Parameters();

    protected:
        VideoElement * video_element;
        bool is_loading;

        Gtk::Label preset_text, crf_text, cpu_text, q_text, ns_text, t_text;
        Gtk::Label preset_caption, crf_caption, cpu_caption, q_caption, ns_caption, t_caption;
        Gtk::Box preset_hbox, crf_hbox, cpu_hbox, q_hbox, ns_hbox, t_hbox;
        Gtk::Box preset_vbox, crf_vbox, cpu_vbox, q_vbox, ns_vbox, t_vbox; 
        Gtk::SpinButton cpu_w, ns_w, preset_w, crf_w;
        Gtk::Scale q_w, t_w;

        void load();
        void update();
        bool on_move_slider(Gtk::ScrollType, double);
};

// Stránka pro označené video ve frontě. Obsahuje základní nastavení pro každé video individuálně
class VideoSettings_VBox : public Gtk::ScrolledWindow
{
    public:
        VideoSettings_VBox();
        ~VideoSettings_VBox();

        // Aktualizace nastavení videa
        void read_video_options(VideoElement * video);
        void read_video_vector_options(std::vector<VideoElement *> video_vector);
        void no_video_selected();

    protected:
        // Buď se bude dělat operace na jednom označeném videu, nebo na všech najednou
        VideoElement * video_element;
        std::vector<VideoElement *> video_queue;
        bool batch_settings;
        bool is_loading;
        string output_path;

        Gtk::Box window_content;

        // Režim kódování
        Gtk::CheckButton compress_mode_radio_button;
        Gtk::CheckButton archive_mode_radio_button;
        Gtk::Label compress_label;
        Gtk::Label archive_label;
        Gtk::Label compress_caption;
        Gtk::Label archive_caption;
        Gtk::Label mode_heading;
        Gtk::Box archive_mode_text_vbox;
        Gtk::Box archive_mode_hbox;
        Gtk::Box compress_mode_text_vbox;
        Gtk::Box compress_mode_hbox;
        Gtk::Box mode_heading_hbox;
        Gtk::ListBox mode_listbox;
        Gtk::Button mode_desc_trigger;
        Gtk::Popover mode_desc;
        Gtk::Label mode_desc_text;

        // Kodek
        Gtk::ToggleButton codec_av1_toggle;
        Gtk::ToggleButton codec_hevc_toggle;
        Gtk::ToggleButton codec_vp9_toggle;
        Gtk::FlowBox codec_flowbox;
        Gtk::Box codec_heading_hbox;
        Gtk::Label codec_heading;
        Gtk::Button codec_desc_trigger;
        Gtk::Popover codec_desc;
        Gtk::Label codec_desc_text;
        Gtk::Switch two_pass_check;
        Gtk::Label two_pass_label;

        // Cílová velikost
        Gtk::Box target_size_hbox;
        Gtk::Label target_size_label;
        Gtk::Label target_size_unit;
        Glib::RefPtr<Gtk::Adjustment> target_size_values;
        Gtk::SpinButton target_size_field;

        // Střih
        Gtk::Label cut_heading;
        Gtk::Button cut_desc_trigger;
        Gtk::Popover cut_desc;
        Gtk::Label cut_desc_text;
        Gtk::Switch cut_switch;
        Gtk::Box cut_heading_hbox, cut_switch_box, cut_switch_text_vbox, cut_start_h_box, cut_start_m_box, cut_start_s_box, cut_stop_h_box, cut_stop_m_box, cut_stop_s_box;
        Gtk::Box cut_start_box, cut_stop_box, cut_start_time_box, cut_stop_time_box;
        Gtk::Label cut_start_text, cut_stop_text, cut_h_text, cut_h2_text, cut_m_text, cut_m2_text, cut_s_text, cut_s2_text, cut_switch_text, cut_switch_desc;
        Gtk::SpinButton cut_start_h, cut_start_m, cut_start_s; 
        Gtk::SpinButton cut_stop_h, cut_stop_m, cut_stop_s;
        Gtk::ListBox cut_listbox;
        Glib::RefPtr<Gtk::Adjustment> lim_start_h, lim_start_m, lim_start_s, lim_stop_h, lim_stop_m, lim_stop_s;

        // Fps a rozlišení
        Gtk::Box res_hbox, res_text_vbox, fps_hbox, fps_text_vbox;
        Gtk::Label res_text, fps_text, res_caption, fps_caption;
        Gtk::SpinButton res_field, fps_field;

        // Výstupní složka a prefix
        Gtk::Label output_heading;
        Gtk::Button output_desc_trigger;
        Gtk::Popover output_desc;
        Gtk::Label output_desc_text;
        Gtk::Box output_heading_hbox, output_hbox, output_text_vbox, prefix_hbox, prefix_text_vbox;
        Gtk::Label output_text, output_caption, prefix_text, prefix_caption;
        Gtk::Button set_output_folder_button;
        Gtk::Entry set_prefix_field;
        Gtk::ListBox output_listbox;

        // Nastavení parametrů kodeků
        Gtk::Label parameters_heading;
        Gtk::Label parameters_desc_text;
        Gtk::Button parameters_desc_trigger;
        Gtk::Box parameters_heading_hbox;
        Gtk::Popover parameters_desc;

        void load_options_into_GUI(Video * video);
        void update();
        void save_options(VideoElement * element);
        void on_select_row(Gtk::ListBoxRow * selected_row);
        void on_select_flowbox(Gtk::FlowBoxChild * child);
        void set_output_path();
        void on_folder_selected(Glib::RefPtr<Gio::AsyncResult> &result, Glib::RefPtr<Gtk::FileDialog> folder_picker);
        void switch_codec_page(Codec codec);
    };


class ResultsDialog : public Gtk::Dialog
{
    public:
        ResultsDialog(Gtk::Window& parent, const std::vector<EncodingResult>& encoding_results);
        ~ResultsDialog();
    
    protected:
        std::vector<EncodingResult> encoding_results;

        Gtk::ScrolledWindow scrolled_window;
        Gtk::ListBox results_listbox;
        Gtk::Box window_content;
        Gtk::Button ok_button;

        void load_results();
};

class MainWindow : public Gtk::Window
{
    public:
        MainWindow();
        ~MainWindow();

    protected:
        Gtk::HeaderBar header_bar;
        RunnerPanel runner_panel;
        QueueFrame video_queue;
        VideoSettings_VBox options_page;
        Gtk::Paned paned;

        // Vlákno pro kódování videí, synchronizace
        std::thread encoding_thread;
        std::atomic<bool> is_encoding;
        std::mutex encoding_mutex;

        // Dispatcher vláken
        Glib::Dispatcher progress_dispatcher;
        Glib::Dispatcher completion_dispatcher;

        EncodingProgress current_progress;
        std::vector<EncodingResult> encoding_results;

        // Kódování
        void start_encoding();
        void stop_encoding();
        void encoding_worker();
        void on_progress_update();
        void on_encoding_complete();
        void show_results_dialog();
};

#endif