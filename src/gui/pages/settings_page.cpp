#include "adwaita.h"
#include "giomm/file.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtkmm/box.h"
#include "gtkmm/enums.h"
#include "gtkmm/error.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include "../headers/gui.h"
#include "sigc++/adaptors/bind.h"
#include "sigc++/functors/mem_fun.h"
#include "src/video/video.h"
#include <filesystem>
#include <iostream>
#include <vector>

SettingsPage::SettingsPage()
:   video_element(nullptr),
    video_queue(),
    batch_settings(false),
    is_loading(false),
    output_path(""),
    
    // Režim
    mode_heading("Encoding Mode"), 
    compress_row("Compress", "Compresses the video to a target size. "),
    archive_row("Archive", "Makes the video a small as possible without loosing quality. "), 
    
    // Kodek
    target_size_row("Target size: ", "Sets the size the video will be compressed to in MB"),
    res_row("Downscale Factor", "Defines how many times smaller is the output resolution. "),
    fps_row("Framerate", "Sets the output framerate. "),

    // Střih
    cut_heading("Cut Feature"),
    cut_switch("Enable Cut", "Enables a feature that trims the video from start time to end time. "),

    // Výstup
    output_heading("Saving"),
    output_row("Saving Destination", "Video(s) will be saved to: \n"),
    prefix_row("File Prefix", "Prepends a text before video name(s). "),
    
    codec_heading("Video Codec")
{
    window_content.set_expand();
    window_content.set_margin(50);
    window_content.set_halign(Gtk::Align::CENTER);
    window_content.set_orientation(Gtk::Orientation::VERTICAL);
    set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    set_child(window_content);
    set_expand();

    // Režim kódování
    string mode_help = 
        "<big>Encoding Mode</big>\n"
        "There are two modes for encoding: \n\n"
        "- <b>ARCHIVE mode</b>, which compresses video(s) as much as possible without losing quality. (DEFAULT)\n"
        "- <b>COMPRESS mode</b>, which attempts to compress video(s) to the target size. \n\n"
        "\tIn COMPRESS mode, you can set:\n"
        "\t- <b>Target size</b> for the output in MB. \n"
        "\t- <b>Downscaling factor</b> - defines how many times smaller is the output resolution. 1.0 means the original resolution is preserved. \n"
        "\t- <b>Framerate</b> - sets the output framerate. \n\n"
        "When using COMPRESS mode, sometimes the resulting videos can be cut off too early. \n"
        "This happens because the target file size is reached before the encoding is finished. \n"
        "If this happens to you, consider lowering the resolution and framerate. ";

    mode_heading.set_popover_contents(mode_help);
    archive_row.set_group(compress_row);
    target_size_row.set_digits(1);
    target_size_row.set_adjustment(10, 0, 16343, 0.1);
    
    mode_listbox.append(archive_row);
    mode_listbox.append(compress_row);
    mode_listbox.append(target_size_row);

    // Střih
    string cut_help = 
        "<big>Cut Feature</big>\n"
        "Trims the video from start time to end time. ";

    cut_heading.set_popover_contents(cut_help);

    cut_listbox.append(cut_switch);
    cut_listbox.append(cut_widget);
    cut_listbox.set_row_active(1, false);

    // Rozlišení a fps
    res_row.set_digits(1);
    fps_row.set_digits(0);
    res_row.set_adjustment(1, 1, 10, 0.1);
    fps_row.set_step(1);

    mode_listbox.append(res_row);
    mode_listbox.append(fps_row);

    // Výstup a prefix
    string output_help = 
        "<big>Saving</big>\n"
        "You can set the output folder for you video(s) by clicking the button below. \n"
        "The app will create a subfolder called 'encoded_videos' in the output folder and save the result there. \n"
        "<b>Prefix</b> field lets you set a string that will be prepended before the original filename(s). ";

    output_heading.set_popover_contents(output_help);
    output_row.set_button_text("Set output folder");
    prefix_row.set_field_placeholder_text("Enter file prefix");

    output_listbox.append(prefix_row);
    output_listbox.append(output_row);
    output_listbox.set_row_active(0, false);

    // Kodek
    string codec_help = 
        "<big>Video Codec</big>\n"
        "Sets the codec used for encoding video(s). \n\n"
        "- <b>AV1</b> - Open source codec. Has the best compression efficiency, but may be more demanding on hardware. DEFAULT\n"
        "- <b>HEVC</b> - Industry standard with great compression. \n"
        "- <b>VP9</b> - Open source codec from Google, used mainly for videos on the web. \n\n"
        "<big>Advanced Codec Options</big>\n"
        "You can adjust the default settings there to achieve faster encoding time or better quality. \n"
        "Different codecs have different options. Feel free to experiment. \n\n"
        "<big>General recommendations</big>\n"
        "- Adjust the <b>preset</b> value if you want to speed up encoding and you don't mind the potential quality decrease. \n"
        "- If you want to increase quality, lower the <b>CRF</b> value. Keep in mind however that it will increase bitrate. \n"
        "- <b>Film grain synthesis</b> in <b>AV1</b> can be very useful when you have a lot of film grain/noise in your video(s). \n"
        "- <b>Psychovisual tuning</b> can achieve a better look. It puts more data in places where we are more likely to look, exploiting our way of seeing the world. \n";

    codec_heading.set_popover_contents(codec_help);
    
    codec_pages = ADW_VIEW_STACK(adw_view_stack_new());
    codec_switch = ADW_INLINE_VIEW_SWITCHER(adw_inline_view_switcher_new());
    adw_view_stack_add_titled(codec_pages, GTK_WIDGET(av1_page.gobj()), "codec_av1", "AV1");
    adw_view_stack_add_titled(codec_pages, GTK_WIDGET(hevc_page.gobj()), "codec_hevc", "HEVC");
    adw_view_stack_add_titled(codec_pages, GTK_WIDGET(vp9_page.gobj()), "codec_vp9", "VP9");
    adw_view_stack_add_titled(codec_pages, GTK_WIDGET(avc_page.gobj()), "codec_avc", "AVC");
    adw_view_stack_set_vhomogeneous(codec_pages, false);
    adw_inline_view_switcher_set_stack(codec_switch, codec_pages);
    
    gtk_widget_add_css_class(GTK_WIDGET(codec_switch), "round");
    codec_listbox.append(*Glib::wrap(GTK_WIDGET(codec_switch)));
    codec_listbox.append(*Glib::wrap(GTK_WIDGET(codec_pages)));
    codec_listbox.set_row_active(0, false);
    codec_listbox.set_row_active(1, false);

    // Složení obsahu okna
    window_content.append(mode_heading);
    window_content.append(mode_listbox);
    window_content.append(cut_heading);
    window_content.append(cut_listbox);
    window_content.append(output_heading);
    window_content.append(output_listbox);
    window_content.append(codec_heading);
    window_content.append(codec_listbox);

    // Signál při změně jakéhokoliv prvku
    archive_row.signal_toggled.connect([this](){ update(&SettingsPage::save_archive_mode); });
    compress_row.signal_toggled.connect([this](){ update(&SettingsPage::save_compress_mode); });
    target_size_row.signal_value_changed.connect([this](){ update(&SettingsPage::save_target_size); });
    fps_row.signal_value_changed.connect([this](){ update(&SettingsPage::save_target_fps); });
    res_row.signal_value_changed.connect([this](){ update(&SettingsPage::save_target_res); });
    cut_switch.signal_toggled.connect([this](){ update(&SettingsPage::save_cut); });
    cut_widget.signal_cut_change.connect([this](){ update(&SettingsPage::save_cut); });
    prefix_row.signal_changed.connect([this](){ update(&SettingsPage::save_prefix); });
    output_row.signal_clicked.connect(sigc::mem_fun(*this, &SettingsPage::set_output_path));
    g_signal_connect(codec_pages, "notify::visible-child", 
        G_CALLBACK(+[](AdwViewStack *, GParamSpec *, gpointer data)
            {
                SettingsPage * self = static_cast<SettingsPage *>(data);
                self -> update(&SettingsPage::save_codec);
            }),
        this);
}

SettingsPage::~SettingsPage()
{}

void SettingsPage::set_output_path()
{
    auto folder_picker = Gtk::FileDialog::create();
    folder_picker -> set_title("Select Output Folder");
    folder_picker -> set_modal();
    auto current_folder = Gio::File::create_for_path(output_path);
    folder_picker -> set_initial_folder(current_folder);
    folder_picker -> select_folder(* dynamic_cast<Gtk::Window *>(get_root()), sigc::bind(sigc::mem_fun(*this, &SettingsPage::on_folder_selected), folder_picker));
}

void SettingsPage::on_folder_selected(Glib::RefPtr<Gio::AsyncResult> &result, Glib::RefPtr<Gtk::FileDialog> folder_picker)
{
    try
    {
        auto folder = folder_picker -> select_folder_finish(result);

        if (folder)
        {
            output_path = folder -> get_path();
            update(&SettingsPage::save_output_path);
        }
    }
    catch (const Gtk::DialogError& error)
    {
        if (error.code() != Gtk::DialogError::DISMISSED)
        {
            cerr << "Folder picker cancelled by user." << endl;
        }
    }
    catch (const Glib::Error& error)
    {
        cerr << "Error selecting folder: " << error.what() << endl;
        dynamic_cast<MainWindow *>(get_root()) -> show_toast("Error selecting folder!");
    }
}

// Ukládací funkce
void SettingsPage::save_archive_mode(VideoElement * element)
{
    element -> video.set_compress(false);
    target_size_row.set_sensitive(false);
    fps_row.set_sensitive(false);
    res_row.set_sensitive(false);
}

void SettingsPage::save_compress_mode(VideoElement * element)
{
    element -> video.set_compress(true);
    target_size_row.set_sensitive();
    fps_row.set_sensitive();
    res_row.set_sensitive();
}

void SettingsPage::save_target_size(VideoElement * element)
{
    float target_size = target_size_row.get_value();
    element -> video.set_bitrate_by_size(target_size);
}

void SettingsPage::save_target_res(VideoElement * element)
{
    float downscale_factor = res_row.get_value();
    element -> video.set_downscale_factor(downscale_factor);
}

void SettingsPage::save_target_fps(VideoElement * element)
{
    int fps = fps_row.get_value();
    element -> video.set_output_framerate(fps);
}

void SettingsPage::save_cut(VideoElement * element)
{
    bool state = cut_switch.get_state();
    element -> video.enable_cut(state);
    cut_widget.set_sensitive(state);
    
    if (state)
    {
        element -> video.set_cut(
            cut_widget.get_start(), 
            cut_widget.get_end()
        );
    }
    
    element -> video.set_bitrate_by_size(element -> video.get_target_size());
}

void SettingsPage::save_prefix(VideoElement * element)
{
    Glib::ustring prefix = prefix_row.get_field_text();
    element -> video.set_prefix(prefix);
    output_row.set_caption("Video(s) will be saved to: \n" + element -> video.get_output_path());
}

void SettingsPage::save_output_path(VideoElement * element)
{
    element -> video.set_output_path(output_path);
    output_row.set_caption("Video(s) will be saved to: \n" + element -> video.get_output_path());
}

void SettingsPage::save_codec(VideoElement * element)
{
    const char * page_name = adw_view_stack_get_visible_child_name(codec_pages);
    if (string(page_name) == "codec_av1") { element -> video.set_codec(AV1); }
    else if (string(page_name) == "codec_hevc") { element -> video.set_codec(HEVC); }
    else if (string(page_name) == "codec_vp9") { element -> video.set_codec(VP9); }
    else if (string(page_name) == "codec_avc") { element -> video.set_codec(AVC); }
}

void SettingsPage::update(void (SettingsPage::*func)(VideoElement *))
{
    // Nesmí se ukládat, když se načítá
    if (is_loading)
        return;

    if (batch_settings)
        for (VideoElement * element : video_queue)
        {
            (this ->* func)(element);
            element -> update_labels();
        }
    else
    {
        (this ->* func)(video_element);
        video_element -> update_labels();
    }
}

void SettingsPage::load_options_into_GUI(Video * video)
{
    SafeReset safe_reset(is_loading);

    // Režim
    if (video -> is_compress_enabled())
        {
            compress_row.set_state(true);
            mode_listbox.select_row(*mode_listbox.get_row_at_index(1));
            target_size_row.set_sensitive();
            res_row.set_sensitive();
            fps_row.set_sensitive();
        }
    else 
        {
            archive_row.set_state(true);
            mode_listbox.select_row(*mode_listbox.get_row_at_index(0));
            target_size_row.set_sensitive(false);
            res_row.set_sensitive(false);
            fps_row.set_sensitive(false);
        }

    // Kodek
    av1_page.load(video_element);
    hevc_page.load(video_element);
    vp9_page.load(video_element);
    avc_page.load(video_element);
    
    if (batch_settings)
    {
        av1_page.load_vector(video_queue);
        hevc_page.load_vector(video_queue);
        vp9_page.load_vector(video_queue);
    }
    
    switch(video_element -> video.get_codec())
    {
        case AV1:
            adw_view_stack_set_visible_child_name(codec_pages, "codec_av1");
            break;
        
        case HEVC:
            adw_view_stack_set_visible_child_name(codec_pages, "codec_hevc");
            break;
                
        case VP9:
            adw_view_stack_set_visible_child_name(codec_pages, "codec_vp9");
            break;
            
        case AVC:
            adw_view_stack_set_visible_child_name(codec_pages, "codec_avc");
            break;
    }

    // Střih
    cut_switch.set_state(video -> is_cutting_enabled());
    cut_widget.set_sensitive(video -> is_cutting_enabled());
    cut_widget.set_cut(video -> get_video_info().duration, video -> get_cut_info());
    
    // Rozlišení a fps
    target_size_row.set_value(video -> get_target_size());
    res_row.set_value(video -> get_downscale_factor());
    fps_row.set_range(0, video -> get_video_info().framerate);
    fps_row.set_value(video -> get_output_framerate());

    // Výstup a prefix
    prefix_row.set_field_text(video -> get_prefix());
    output_row.set_caption("Video(s) will be saved to: \n" + video -> get_output_path());
}

void SettingsPage::read_video_vector_options(std::vector<VideoElement *> video_vector)
{
    this -> video_queue = video_vector;
    if (!batch_settings)
    {
        batch_settings = true;
        cut_heading.add_css_class("warning");
        cut_heading.set_heading("Cut Feature (set according to the shortest video)");
    }

    if (video_queue.size() > 0)
    {
        // Volba videa pro načtení do GUI: 
        //  Najde se video s nejkratší délkou (kvůli funkci střihu) a podle něho se načtou nastavení do GUI
        //  To bude užitečné například v případech, kdy má uživatel několik stejně dlouhých videí a chce překódovat prvních pár minut každého videa
        float min_duration = MAXFLOAT;
        int video_index = -1;
        
        for (unsigned long i = 0; i < video_queue.size(); i++)
        {
            if (min_duration > video_queue.at(i) -> video.get_video_info().duration)
            {
                video_index = i;
                min_duration = video_queue.at(i) -> video.get_video_info().duration;
            }
        }
        
        Video * video = &(video_queue.at(video_index) -> video);
        output_path = video -> get_output_path();
        output_path = filesystem::path(output_path).parent_path().generic_string().substr(0, output_path.find("encoded_videos/"));
        load_options_into_GUI(video);
    }
}

void SettingsPage::read_video_options(VideoElement * video_element)
{
    this -> video_element = video_element;
    Video * video = &(video_element -> video);
    output_path = video -> get_output_path();
    output_path = filesystem::path(output_path).parent_path().generic_string().substr(0, output_path.find("encoded_videos/"));
    
    if (batch_settings)
    {
        cut_heading.remove_css_class("warning");
        cut_heading.set_heading("Cut Feature");
        batch_settings = false;
    }

    load_options_into_GUI(video);
}
