#include "giomm/file.h"
#include "glibmm/refptr.h"
#include "gtkmm/box.h"
#include "gtkmm/enums.h"
#include "gtkmm/error.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/object.h"
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
    codec_heading("Video Codec"),
    codec_av1_toggle("AV1"),
    codec_hevc_toggle("HEVC"),

    codec_vp9_toggle("VP9"),

    target_size_row("Target size: ", "Sets the size the video will be compressed to in MB"),

    // Střih
    cut_heading("Cut Feature"),
    cut_switch("Enable Cut", "Enables a feature that trims the video from start time to end time. "),
    // Rozlišení a fps
    res_row("Downscale Factor", "Defines how many times smaller is the output resolution. "),
    fps_row("Framerate", "Sets the output framerate. "),
    // Výstup
    output_heading("Saving"),
    output_button("Saving Destination", "Video(s) will be saved to: \n"),
    prefix_row("File Prefix", "Prepends a text before video name(s). "),
    // Parametry
    parameter_heading("Advanced Codec Options")
{
    window_content.set_expand();
    window_content.set_margin(50);
    window_content.set_halign(Gtk::Align::CENTER);
    window_content.set_orientation(Gtk::Orientation::VERTICAL);
    set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    set_child(window_content);
    set_expand();
    set_sensitive(false);

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

    // Kodek
    string codec_help = 
        "<big>Video Codec</big>\n"
        "Sets the codec used for encoding video(s). \n\n"
        "- <b>AV1</b> - Open source codec. Has the best compression efficiency, but may be more demanding on hardware. DEFAULT\n"
        "- <b>HEVC</b> - Industry standart with great compression. \n"
        "- <b>VP9</b> - Open source codec from Google, used mainly for videos on the web. ";

    codec_heading.set_popover_contents(codec_help);

    codec_hevc_toggle.set_group(codec_av1_toggle);
    codec_vp9_toggle.set_group(codec_av1_toggle);
    codec_av1_toggle.set_can_target(false);
    codec_hevc_toggle.set_can_target(false);
    codec_vp9_toggle.set_can_target(false);

    codec_flowbox.add_css_class("navigation-sidebar");
    codec_flowbox.add_css_class("card");
    codec_flowbox.set_orientation(Gtk::Orientation::HORIZONTAL);
    codec_flowbox.set_halign(Gtk::Align::CENTER);
    codec_flowbox.set_margin_bottom(20);
    codec_flowbox.set_min_children_per_line(3);
    codec_flowbox.append(codec_av1_toggle);
    codec_flowbox.append(codec_hevc_toggle);
    codec_flowbox.append(codec_vp9_toggle);

    // Střih
    string cut_help = 
        "<big>Cut Feature</big>\n"
        "Trims the video from start time to end time. \n"
        "This feature cannot be used with all videos selected. ";

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
        "The program will create a subfolder called 'encoded_videos' in the output folder and save the result there. \n"
        "<b>Prefix</b> field lets you set a string that will be prepended before the original filename(s). ";

    output_heading.set_popover_contents(output_help);
    output_button.set_button_text("Set output folder");
    
    prefix_row.set_field_placeholder_text("Enter file prefix");

    output_listbox.append(prefix_row);
    output_listbox.append(output_button);
    output_listbox.set_row_active(0, false);

    // Parametry
    string parameters_help = 
        "<big>Advanced Codec Options</big>\n"
        "You can adjust the default settings there to achieve faster encoding time or better quality. \n Different codecs have different options. Feel free to experiment. \n\n"
        "<big>General recommendations</big>\n"
        "- Adjust the <b>preset</b> value if you want to speed up encoding and you don't mind the potential quality decrease. \n"
        "- If you want to increase quality, lower the <b>CRF</b> value. Keep in mind however that it will increase bitrate. \n"
        "- <b>Film grain synthesis</b> in <b>AV1</b> can be very useful when you have a lot of film grain/noise in your video(s). \n"
        "- <b>Psychovisual tuning</b> can achieve a better look. It puts more data in places where we are more likely to look, exploiting our way of seeing the world. \n";

    parameter_heading.set_popover_contents(parameters_help);

    // Složení obsahu okna
    window_content.append(mode_heading);
    window_content.append(mode_listbox);
    window_content.append(cut_heading);
    window_content.append(cut_listbox);
    window_content.append(output_heading);
    window_content.append(output_listbox);
    window_content.append(codec_heading);
    window_content.append(codec_flowbox);
    window_content.append(parameter_heading);
    window_content.append(*Gtk::make_managed<AV1_Parameters>(video_element));

    // Signál při změně jakéhokoliv prvku
    archive_row.signal_toggled.connect([this](){ update(&SettingsPage::save_all_options); });
    compress_row.signal_toggled.connect([this](){ update(&SettingsPage::save_all_options); });
    // mode_listbox.signal_row_selected().connect([this](){ update(&SettingsPage::save_all_options); });
    target_size_row.signal_value_changed.connect([this](){ update(&SettingsPage::save_all_options); });
    codec_flowbox.signal_child_activated().connect(sigc::mem_fun(*this, &SettingsPage::on_select_flowbox));
    cut_switch.signal_toggled.connect([this](){ update(&SettingsPage::save_cut); });
    // cut_listbox.signal_row_activated().connect(sigc::mem_fun(*this, &SettingsPage::on_select_row));
    cut_widget.signal_cut_change.connect([this](){ update(&SettingsPage::save_all_options); });
    prefix_row.signal_changed.connect([this](){ update(&SettingsPage::save_all_options); });
    output_button.signal_clicked.connect(sigc::mem_fun(*this, &SettingsPage::set_output_path));
    fps_row.signal_value_changed.connect([this](){ update(&SettingsPage::save_all_options); });
    res_row.signal_value_changed.connect([this](){ update(&SettingsPage::save_all_options); });
}

SettingsPage::~SettingsPage()
{}

void SettingsPage::switch_codec_page(Codec codec)
{
    window_content.remove(*window_content.get_last_child());

    // Podle kodeků zařazuji příslušné stránky na místo té předchozí
    // Díky friend jsou stránky odsud viditelné a můžu s nimi pracovat, což je potřebné
    // Zavádím zde definici funkce stránky, která zaktualizuje i zbytek nastavení při změně ve stránce, 
    // čímž se tyto stránky hezky propojí. Šlo by to udělat i přes signály, ale tohle se zdá být čistší postup. 
    
    switch (codec)
    {
        case AV1:
        {
            window_content.append(*Gtk::make_managed<AV1_Parameters>(video_element));
            auto page = dynamic_cast<AV1_Parameters *>(window_content.get_last_child());
            page -> on_updated = [this]() { update(&SettingsPage::save_all_options); };
            page -> load();
            break;
        }

        case HEVC:
        {
            window_content.append(*Gtk::make_managed<HEVC_Parameters>(video_element));
            auto page = dynamic_cast<HEVC_Parameters *>(window_content.get_last_child());
            page -> on_updated = [this]() { update(&SettingsPage::save_all_options); };
            page -> load();
            break;
        }

        case VP9:
        {
            window_content.append(*Gtk::make_managed<VP9_Parameters>(video_element));
            auto page = dynamic_cast<VP9_Parameters *>(window_content.get_last_child());
            page -> on_updated = [this]() { update(&SettingsPage::save_all_options); };
            page -> load();
            break;
        }
    }

}

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
            update(&SettingsPage::save_all_options);
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

void SettingsPage::on_select_flowbox(Gtk::FlowBoxChild * child)
{
    // Kodek
    if (child -> is_ancestor(codec_flowbox))
    {
        auto av1 = codec_flowbox.get_child_at_index(0);
        auto hevc = codec_flowbox.get_child_at_index(1);
        auto vp9 = codec_flowbox.get_child_at_index(2);

        if (child == av1)
        {
            codec_av1_toggle.set_active();
        }
        else if (child == hevc) 
        {
            codec_hevc_toggle.set_active();
        }
        else if (child == vp9) 
        {
            codec_vp9_toggle.set_active();
        }
    }

    update(&SettingsPage::save_all_options);
}

void SettingsPage::save_cut(VideoElement * element)
{
    bool state = cut_switch.get_state();
    element -> video.enable_cut(state);
    cut_widget.set_sensitive(state);
}

void SettingsPage::save_all_options(VideoElement * element)
{
    Video * video = &(element -> video);

    // Režim
    if (compress_row.get_state())
        {
            video -> set_compress(true);
            video -> set_bitrate_by_size(target_size_row.get_value());

            // Rozlišení a fps
            video -> set_downscale_factor(res_row.get_value());
            video -> set_output_framerate(fps_row.get_value());
        }
    else
        {
            video -> set_compress(false);
        }
       
    // Kodek

    Codec new_codec_setting;
    
    if (codec_av1_toggle.get_active()) new_codec_setting = AV1;
    else if (codec_hevc_toggle.get_active()) new_codec_setting = HEVC;
    else new_codec_setting = VP9;
    
    if (video -> get_codec() != new_codec_setting)
    {
        video -> set_codec(new_codec_setting);
        switch_codec_page(new_codec_setting);
    }
    
    // Střih
    if (cut_switch.get_state())
    {
        video -> enable_cut(true);
        video -> set_cut(cut_widget.get_start(), cut_widget.get_end());
    }
    else video -> enable_cut(false);
    
    // Výstup a prefix
    video -> set_output_path(output_path);
    video -> set_prefix(prefix_row.get_field_text());
    output_button.set_caption("Video(s) will be saved to: \n" + video -> get_output_path());
    
    // Parametry kodeku
    if (batch_settings)
    {
        Codec current_codec = video -> get_codec();
        
        if (video_element)
        {
            Video * displayed_video = &(video_element -> video);
            
            if (current_codec == AV1) video -> AV1_options = displayed_video -> AV1_options;
            else if (current_codec == HEVC) video -> HEVC_options = displayed_video -> HEVC_options;
            else video -> VP9_options = displayed_video -> VP9_options;
        }
    }
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
    Codec codec = video -> get_codec();

    // Režim
    if (video -> is_compress_enabled())
        {
            compress_row.set_state(true);
            mode_listbox.select_row(*mode_listbox.get_row_at_index(1));
            target_size_row.set_value(video -> get_target_size());
            target_size_row.set_sensitive();
            res_row.set_sensitive();
            fps_row.set_sensitive();
        }
    else 
        {
            archive_row.set_state(true);
            mode_listbox.select_row(*mode_listbox.get_row_at_index(0));
            // target_size_row.set_sensitive(false);
            // res_hbox.set_sensitive(false);
            // fps_hbox.set_sensitive(false);
        }

    // Kodek
    if (codec == AV1)
    {
        codec_av1_toggle.set_active();
        codec_flowbox.select_child(*codec_flowbox.get_child_at_index(0));
        switch_codec_page(AV1);
    }
    else if (codec == HEVC)
    {
        codec_hevc_toggle.set_active();
        codec_flowbox.select_child(*codec_flowbox.get_child_at_index(1));
        switch_codec_page(HEVC);
    }
    else if (codec == VP9) 
    {
        codec_vp9_toggle.set_active();
        codec_flowbox.select_child(*codec_flowbox.get_child_at_index(2));
        switch_codec_page(VP9);
    }

    // Střih
    cut_switch.set_state(video -> is_cutting_enabled());
    cut_widget.set_sensitive(video -> is_cutting_enabled());
    cut_widget.set_cut(video -> get_video_info().duration, video -> get_cut_info());
    
    // Rozlišení a fps
    res_row.set_value(video -> get_downscale_factor());
    fps_row.set_range(0, video -> get_video_info().framerate);
    fps_row.set_value(video -> get_output_framerate());

    // Výstup a prefix
    prefix_row.set_field_text(video -> get_prefix());
    output_button.set_caption("Video(s) will be saved to: \n" + video -> get_output_path());

    set_sensitive();
}

void SettingsPage::read_video_vector_options(std::vector<VideoElement *> video_vector)
{
    this -> video_queue = video_vector ;
    batch_settings = true;
    cut_heading.add_css_class("warning");
    cut_heading.set_heading("Cut Feature (set according to the shortest video)");

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
    batch_settings = false;

    cut_heading.remove_css_class("warning");
    cut_heading.set_heading("Cut Feature");

    load_options_into_GUI(video);
}

void SettingsPage::no_video_selected()
{
    set_sensitive(false);
}
