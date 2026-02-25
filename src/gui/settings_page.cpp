#include "giomm/file.h"
#include "glibmm/refptr.h"
#include "gtkmm/adjustment.h"
#include "gtkmm/box.h"
#include "gtkmm/enums.h"
#include "gtkmm/error.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/label.h"
#include "gtkmm/listboxrow.h"
#include "gtkmm/object.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include "gui.h"
#include "pangomm/layout.h"
#include "sigc++/adaptors/bind.h"
#include "sigc++/functors/mem_fun.h"
#include "src/video/video.h"
#include <filesystem>
#include <iostream>
#include <vector>

VideoSettings_VBox::VideoSettings_VBox()
:   video_element(nullptr),
    video_queue(),
    batch_settings(false),
    is_loading(false),
    output_path(""),
    // Režim
    compress_label("Compress", Gtk::Align::START), 
    archive_label("Archive", Gtk::Align::START),
    compress_caption("Compresses the video to a target size. ", Gtk::Align::START), 
    archive_caption("Makes the video a small as possible without loosing quality. ", Gtk::Align::START),
    mode_heading("Encoding Mode"),
    
    archive_mode_text_vbox(Gtk::Orientation::VERTICAL),
    archive_mode_hbox(Gtk::Orientation::HORIZONTAL),
    compress_mode_text_vbox(Gtk::Orientation::VERTICAL),
    compress_mode_hbox(Gtk::Orientation::HORIZONTAL),
    mode_heading_hbox(Gtk::Orientation::HORIZONTAL),
    // Kodek
    codec_av1_toggle("AV1"),
    codec_hevc_toggle("HEVC"),
    codec_vp9_toggle("VP9"),

    codec_heading_hbox(Gtk::Orientation::HORIZONTAL),
    codec_heading("Video Codec"),

    target_size_hbox(Gtk::Orientation::HORIZONTAL),
    target_size_label("Target size: ", Gtk::Align::START),
    target_size_unit(" MB"),
    target_size_values(Gtk::Adjustment::create(10.0, 0.0, 16343)),
    target_size_field(target_size_values), 
    // Střih
    cut_heading("Cut Feature"),
    cut_heading_hbox(Gtk::Orientation::HORIZONTAL),
    cut_switch_box(Gtk::Orientation::HORIZONTAL),
    cut_switch_text_vbox(Gtk::Orientation::VERTICAL),
    cut_start_h_box(Gtk::Orientation::HORIZONTAL),
    cut_start_m_box(Gtk::Orientation::HORIZONTAL),
    cut_start_s_box(Gtk::Orientation::HORIZONTAL),
    cut_stop_h_box(Gtk::Orientation::HORIZONTAL),
    cut_stop_m_box(Gtk::Orientation::HORIZONTAL),
    cut_stop_s_box(Gtk::Orientation::HORIZONTAL),
    cut_start_text("Start time: "), 
    cut_stop_text("End time:  "),
    cut_h_text(" h\t"), 
    cut_h2_text(" h\t"), 
    cut_m_text(" min\t"),
    cut_m2_text(" min\t"),
    cut_s_text(" s\t"),
    cut_s2_text(" s\t"),
    cut_switch_text("Enable Cut"),
    cut_switch_desc("Enables a feature that trims the video from start time to end time. "),
    lim_start_h(Gtk::Adjustment::create(0, 0, 0)),
    lim_start_m(Gtk::Adjustment::create(0, 0, 0)),
    lim_start_s(Gtk::Adjustment::create(0, 0, 0)),
    lim_stop_h(Gtk::Adjustment::create(0, 0, 0)),
    lim_stop_m(Gtk::Adjustment::create(0, 0, 0)),
    lim_stop_s(Gtk::Adjustment::create(0, 0, 0)), 
    // Rozlišení a fps
    res_hbox(Gtk::Orientation::HORIZONTAL),
    res_text_vbox(Gtk::Orientation::VERTICAL),
    fps_hbox(Gtk::Orientation::HORIZONTAL),
    fps_text_vbox(Gtk::Orientation::VERTICAL),
    res_text("Downscale Factor"),
    fps_text("Framerate"),
    res_caption("Defines how many times smaller is the output resolution. "),
    fps_caption("Sets the output framerate. "),
    res_field(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(1, 1, 20, 0.1))),
    fps_field(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(60, 0, 60))),
    // Výstup
    output_heading("Saving"),
    output_heading_hbox(Gtk::Orientation::HORIZONTAL),
    output_hbox(Gtk::Orientation::HORIZONTAL),
    output_text_vbox(Gtk::Orientation::VERTICAL),
    prefix_hbox(Gtk::Orientation::HORIZONTAL),
    prefix_text_vbox(Gtk::Orientation::VERTICAL),
    output_text("Saving Destination"),
    output_caption("Video(s) will be saved to: \n"),
    prefix_text("File Prefix"),
    prefix_caption("Prepends a text before video name(s). "),
    set_output_folder_button("Set output folder"), 
    // Parametry
    parameters_heading("Advanced Codec Options"),
    parameters_heading_hbox(Gtk::Orientation::HORIZONTAL)
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

    mode_desc_text.set_markup(mode_help);
    mode_desc.set_child(mode_desc_text);
    mode_desc.set_parent(mode_desc_trigger);
    mode_desc_trigger.signal_clicked().connect([this](){mode_desc.popup();});
    
    mode_desc.set_margin(10);
    mode_heading.add_css_class("title-4");
    mode_desc_trigger.set_icon_name("help-about-symbolic");
    mode_desc_trigger.add_css_class("flat");

    mode_heading_hbox.set_margin(10);
    mode_heading_hbox.set_halign(Gtk::Align::CENTER);
    mode_heading_hbox.append(mode_heading);
    mode_heading_hbox.append(mode_desc_trigger);

    archive_mode_radio_button.set_group(compress_mode_radio_button);
    archive_mode_radio_button.set_can_target(false);
    compress_mode_radio_button.set_can_target(false);

    archive_mode_hbox.set_margin(5);
    archive_mode_text_vbox.set_margin(5);
    archive_label.set_margin(2);
    archive_caption.set_margin(2);
    archive_caption.set_ellipsize(Pango::EllipsizeMode::END);
    archive_label.add_css_class("heading");
    archive_caption.add_css_class("caption");
    archive_mode_text_vbox.append(archive_label);
    archive_mode_text_vbox.append(archive_caption);
    archive_mode_hbox.append(archive_mode_radio_button);
    archive_mode_hbox.append(archive_mode_text_vbox);

    compress_mode_hbox.set_margin(5);
    compress_mode_text_vbox.set_margin(5);
    compress_label.set_margin(2);
    compress_caption.set_margin(2);
    compress_caption.set_ellipsize(Pango::EllipsizeMode::END);
    compress_label.add_css_class("heading");
    compress_caption.add_css_class("caption");
    compress_mode_text_vbox.append(compress_label);
    compress_mode_text_vbox.append(compress_caption);
    compress_mode_hbox.append(compress_mode_radio_button);
    compress_mode_hbox.append(compress_mode_text_vbox);

    target_size_field.set_numeric();
    target_size_field.set_digits(1);
    target_size_field.set_expand();
    target_size_hbox.set_margin(5);
    target_size_hbox.set_sensitive(false);
    target_size_label.add_css_class("heading");
    target_size_hbox.append(target_size_label);
    target_size_hbox.append(target_size_field);
    target_size_hbox.append(target_size_unit);

    mode_listbox.add_css_class("navigation-sidebar");
    mode_listbox.add_css_class("card");
    mode_listbox.set_margin_bottom(20);
    mode_listbox.append(archive_mode_hbox);
    mode_listbox.append(compress_mode_hbox);
    mode_listbox.append(target_size_hbox);
    mode_listbox.get_row_at_index(2 )-> set_activatable(false);
    mode_listbox.get_row_at_index(2 )-> set_selectable(false);

    // Kodek
    string codec_help = 
        "<big>Video Codec</big>\n"
        "Sets the codec used for encoding video(s). \n\n"
        "- <b>AV1</b> - Open source codec. Has the best compression efficiency, but may be more demanding on hardware. DEFAULT\n"
        "- <b>HEVC</b> - Industry standart with great compression. \n"
        "- <b>VP9</b> - Open source codec from Google, used mainly for videos on the web. ";

    codec_desc_text.set_markup(codec_help);
    codec_desc.set_child(codec_desc_text);
    codec_desc.set_parent(codec_desc_trigger);
    codec_desc_trigger.signal_clicked().connect([this](){codec_desc.popup();});


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

    codec_heading.add_css_class("title-4");
    codec_desc_trigger.set_icon_name("help-about-symbolic");
    codec_desc_trigger.add_css_class("flat");
    codec_heading_hbox.set_margin(10);
    codec_heading_hbox.set_halign(Gtk::Align::CENTER);
    codec_heading_hbox.append(codec_heading);
    codec_heading_hbox.append(codec_desc_trigger);

    // Střih
    string cut_help = 
        "<big>Cut Feature</big>\n"
        "Trims the video from start time to end time. \n"
        "This feature cannot be used with all videos selected. ";

    cut_desc_text.set_markup(cut_help);
    cut_desc.set_child(cut_desc_text);
    cut_desc.set_parent(cut_desc_trigger);
    cut_desc_trigger.signal_clicked().connect([this](){cut_desc.popup();});

    cut_heading.add_css_class("title-4");
    cut_desc_trigger.set_icon_name("help-about-symbolic");
    cut_desc_trigger.add_css_class("flat");
    cut_heading_hbox.set_margin(10);
    cut_heading_hbox.set_halign(Gtk::Align::CENTER);
    cut_heading_hbox.append(cut_heading);
    cut_heading_hbox.append(cut_desc_trigger);

    cut_switch_box.set_margin(5);
    cut_switch_text_vbox.set_margin(5);
    cut_switch_text.set_margin(2);
    cut_switch_desc.set_margin(2);
    cut_switch_desc.set_ellipsize(Pango::EllipsizeMode::END);
    cut_switch_text.add_css_class("heading");
    cut_switch_text.set_halign(Gtk::Align::START);
    cut_switch_desc.add_css_class("caption");
    cut_switch_desc.set_halign(Gtk::Align::START);
    cut_switch_text_vbox.append(cut_switch_text);
    cut_switch_text_vbox.append(cut_switch_desc);
    cut_switch.set_can_target(false);
    cut_switch.set_halign(Gtk::Align::CENTER);
    cut_switch.set_valign(Gtk::Align::CENTER);
    cut_switch_box.append(cut_switch);
    cut_switch_box.append(cut_switch_text_vbox);

    cut_h_text.set_margin(5);
    cut_m_text.set_margin(5);
    cut_s_text.set_margin(5);
    cut_h2_text.set_margin(5);
    cut_m2_text.set_margin(5);
    cut_s2_text.set_margin(5);

    cut_start_h.set_numeric();
    cut_start_m.set_numeric();
    cut_start_s.set_numeric();
    cut_stop_h.set_numeric();
    cut_stop_m.set_numeric();
    cut_stop_s.set_numeric();

    cut_start_box.set_margin(5);
    cut_start_text.add_css_class("heading");
    cut_start_text.set_margin(5);
    cut_start_h_box.append(cut_start_h);
    cut_start_h_box.append(cut_h_text);
    cut_start_m_box.append(cut_start_m);
    cut_start_m_box.append(cut_m_text);
    cut_start_s_box.append(cut_start_s);
    cut_start_s_box.append(cut_s_text);
    cut_start_time_box.set_expand();
    cut_start_time_box.set_halign(Gtk::Align::END);
    cut_start_time_box.append(cut_start_h_box);
    cut_start_time_box.append(cut_start_m_box);
    cut_start_time_box.append(cut_start_s_box);
    cut_start_box.append(cut_start_text);
    cut_start_box.append(cut_start_time_box);
    cut_start_box.set_sensitive(false);
    
    cut_stop_box.set_margin(5);
    cut_stop_text.add_css_class("heading");
    cut_stop_text.set_margin(5);
    cut_stop_h_box.append(cut_stop_h);
    cut_stop_h_box.append(cut_h2_text);
    cut_stop_m_box.append(cut_stop_m);
    cut_stop_m_box.append(cut_m2_text);
    cut_stop_s_box.append(cut_stop_s);
    cut_stop_s_box.append(cut_s2_text);
    cut_stop_time_box.set_expand();
    cut_stop_time_box.set_halign(Gtk::Align::END);
    cut_stop_time_box.append(cut_stop_h_box);
    cut_stop_time_box.append(cut_stop_m_box);
    cut_stop_time_box.append(cut_stop_s_box);
    cut_stop_box.append(cut_stop_text);
    cut_stop_box.append(cut_stop_time_box);
    cut_stop_box.set_sensitive(false);

    cut_listbox.add_css_class("navigation-sidebar");
    cut_listbox.add_css_class("card");
    cut_listbox.set_margin_bottom(20);
    cut_listbox.append(cut_switch_box);
    cut_listbox.append(cut_start_box);
    cut_listbox.append(cut_stop_box);
    cut_listbox.get_row_at_index(1) -> set_activatable(false);
    cut_listbox.get_row_at_index(1) -> set_selectable(false);
    cut_listbox.get_row_at_index(2) -> set_activatable(false);
    cut_listbox.get_row_at_index(2) -> set_selectable(false);

    // Rozlišení a fps
    res_text.set_halign(Gtk::Align::START);
    fps_text.set_halign(Gtk::Align::START);
    res_caption.set_halign(Gtk::Align::START);
    fps_caption.set_halign(Gtk::Align::START);
    res_field.set_numeric();
    fps_field.set_numeric();
    res_field.set_digits(1);

    res_hbox.set_margin(5);
    res_text_vbox.set_margin(5);
    res_text.set_margin(2);
    res_caption.set_margin(2);
    res_caption.set_ellipsize(Pango::EllipsizeMode::END);
    res_text.add_css_class("heading");
    res_caption.add_css_class("caption");
    res_text_vbox.append(res_text);
    res_text_vbox.append(res_caption);
    res_field.set_margin(10);
    res_hbox.append(res_field);
    res_hbox.append(res_text_vbox);

    fps_hbox.set_margin(5);
    fps_text_vbox.set_margin(5);
    fps_text.set_margin(2);
    fps_caption.set_margin(2);
    fps_caption.set_ellipsize(Pango::EllipsizeMode::END);
    fps_text.add_css_class("heading");
    fps_caption.add_css_class("caption");
    fps_text_vbox.append(fps_text);
    fps_text_vbox.append(fps_caption);
    fps_field.set_margin(10);
    fps_hbox.append(fps_field);
    fps_hbox.append(fps_text_vbox);

    mode_listbox.append(res_hbox);
    mode_listbox.append(fps_hbox);
    mode_listbox.get_row_at_index(3 )-> set_activatable(false);
    mode_listbox.get_row_at_index(3 )-> set_selectable(false);
    mode_listbox.get_row_at_index(4 )-> set_activatable(false);
    mode_listbox.get_row_at_index(4 )-> set_selectable(false);
    res_hbox.set_sensitive(false);
    fps_hbox.set_sensitive(false);

    // Výstup a prefix
    string output_help = 
        "<big>Saving</big>\n"
        "You can set the output folder for you video(s) by clicking the button below. \n"
        "The program will create a subfolder called 'encoded_videos' in the output folder and save the result there. \n"
        "<b>Prefix</b> field lets you set a string that will be prepended before the original filename(s). ";

    output_desc_text.set_markup(output_help);
    output_desc.set_child(output_desc_text);
    output_desc.set_parent(output_desc_trigger);
    output_desc_trigger.signal_clicked().connect([this](){output_desc.popup();});

    output_heading.add_css_class("title-4");
    output_desc_trigger.set_icon_name("help-about-symbolic");
    output_desc_trigger.add_css_class("flat");
    output_heading_hbox.set_margin(10);
    output_heading_hbox.set_halign(Gtk::Align::CENTER);
    output_heading_hbox.append(output_heading);
    output_heading_hbox.append(output_desc_trigger);

    output_text.set_halign(Gtk::Align::START);
    output_caption.set_halign(Gtk::Align::START);
    prefix_text.set_halign(Gtk::Align::START);
    prefix_caption.set_halign(Gtk::Align::START);

    output_hbox.set_margin(5);
    output_text_vbox.set_margin(5);
    output_text.set_margin(2);
    output_caption.set_margin(2);
    output_caption.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
    output_text.add_css_class("heading");
    output_caption.add_css_class("caption");
    output_text_vbox.append(output_text);
    output_text_vbox.append(output_caption);
    set_output_folder_button.set_margin(10);
    set_output_folder_button.add_css_class("suggested-action");
    output_hbox.append(set_output_folder_button);
    output_hbox.append(output_text_vbox);

    prefix_hbox.set_margin(5);
    prefix_text_vbox.set_margin(5);
    prefix_text.set_margin(2);
    prefix_caption.set_margin(2);
    prefix_caption.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
    prefix_text.add_css_class("heading");
    prefix_caption.add_css_class("caption");
    prefix_text_vbox.append(prefix_text);
    prefix_text_vbox.append(prefix_caption);
    set_prefix_field.set_margin(10);
    set_prefix_field.set_placeholder_text("Enter file prefix");
    prefix_hbox.append(set_prefix_field);
    prefix_hbox.append(prefix_text_vbox);

    output_listbox.add_css_class("navigation-sidebar");
    output_listbox.add_css_class("card");
    output_listbox.set_margin_bottom(20);
    output_listbox.append(prefix_hbox);
    output_listbox.append(output_hbox);
    output_listbox.get_row_at_index(0 )-> set_activatable(false);
    output_listbox.get_row_at_index(0 )-> set_selectable(false);
    output_listbox.get_row_at_index(1 )-> set_activatable(false);
    output_listbox.get_row_at_index(1 )-> set_selectable(false);

    // Parametry
    string parameters_help = 
        "<big>Advanced Codec Options</big>\n"
        "You can adjust the default settings there to achieve faster encoding time or better quality. \n Different codecs have different options. Feel free to experiment. \n\n"
        "<big>General recommendations</big>\n"
        "- Adjust the <b>preset</b> value if you want to speed up encoding and you don't mind the potential quality decrease. \n"
        "- If you want to increase quality, lower the <b>CRF</b> value. Keep in mind however that it will increase bitrate. \n"
        "- <b>Film grain synthesis</b> in <b>AV1</b> can be very useful when you have a lot of film grain/noise in your video(s). \n"
        "- <b>Psychovisual tuning</b> can achieve a better look. It puts more data in places where we are more likely to look, exploiting our way of seeing the world. \n";

    parameters_desc_text.set_markup(parameters_help);
    parameters_desc.set_child(parameters_desc_text);
    parameters_desc.set_parent(parameters_desc_trigger);
    parameters_desc_trigger.signal_clicked().connect([this](){parameters_desc.popup();});


    parameters_heading.add_css_class("title-4");
    parameters_desc_trigger.set_icon_name("help-about-symbolic");
    parameters_desc_trigger.add_css_class("flat");
    parameters_heading_hbox.set_margin(10);
    parameters_heading_hbox.set_halign(Gtk::Align::CENTER);
    parameters_heading_hbox.append(parameters_heading);
    parameters_heading_hbox.append(parameters_desc_trigger);

    // Složení obsahu okna
    window_content.append(mode_heading_hbox);
    window_content.append(mode_listbox);
    window_content.append(codec_heading_hbox);
    window_content.append(codec_flowbox);
    window_content.append(cut_heading_hbox);
    window_content.append(cut_listbox);
    window_content.append(output_heading_hbox);
    window_content.append(output_listbox);
    window_content.append(parameters_heading_hbox);
    window_content.append(*Gtk::make_managed<AV1_Parameters>(video_element));

    // Signál při změně jakéhokoliv prvku
    mode_listbox.signal_row_selected().connect(sigc::mem_fun(*this, &VideoSettings_VBox::on_select_row));
    target_size_field.signal_value_changed().connect(sigc::mem_fun(*this, &VideoSettings_VBox::update));
    codec_flowbox.signal_child_activated().connect(sigc::mem_fun(*this, &VideoSettings_VBox::on_select_flowbox));
    cut_listbox.signal_row_activated().connect(sigc::mem_fun(*this, &VideoSettings_VBox::on_select_row));
    vector<Gtk::SpinButton *> cut_spinbuttons = {&cut_start_h, &cut_start_m, &cut_start_s, &cut_stop_h, &cut_stop_m, &cut_stop_s, &res_field, &fps_field};
    for (auto c_sb : cut_spinbuttons)
    {
        c_sb -> signal_value_changed().connect(sigc::mem_fun(*this, &VideoSettings_VBox::update));
    }
    set_prefix_field.signal_changed().connect(sigc::mem_fun(*this, &VideoSettings_VBox::update));
    set_output_folder_button.signal_clicked().connect(sigc::mem_fun(*this, &VideoSettings_VBox::set_output_path));
}

VideoSettings_VBox::~VideoSettings_VBox()
{}

void VideoSettings_VBox::switch_codec_page(Codec codec)
{
    window_content.remove(*window_content.get_last_child());

    switch (codec)
    {
        case AV1:
            window_content.append(*Gtk::make_managed<AV1_Parameters>(video_element));
            dynamic_cast<AV1_Parameters *>(window_content.get_last_child()) -> load();
            break;

        case HEVC:
            window_content.append(*Gtk::make_managed<HEVC_Parameters>(video_element));
            dynamic_cast<HEVC_Parameters *>(window_content.get_last_child()) -> load();
            break;

        case VP9:
            window_content.append(*Gtk::make_managed<VP9_Parameters>(video_element));
            dynamic_cast<VP9_Parameters *>(window_content.get_last_child()) -> load();
            break;
    }

}

void VideoSettings_VBox::set_output_path()
{
    auto folder_picker = Gtk::FileDialog::create();
    folder_picker -> set_title("Select Output Folder");
    folder_picker -> set_modal();

    auto current_folder = Gio::File::create_for_path(output_path);
    folder_picker -> set_initial_folder(current_folder);

    folder_picker -> select_folder(* dynamic_cast<Gtk::Window *>(get_root()), sigc::bind(sigc::mem_fun(*this, &VideoSettings_VBox::on_folder_selected), folder_picker));
}

void VideoSettings_VBox::on_folder_selected(Glib::RefPtr<Gio::AsyncResult> &result, Glib::RefPtr<Gtk::FileDialog> folder_picker)
{
    try
    {
        auto folder = folder_picker -> select_folder_finish(result);

        if (folder)
        {
            output_path = folder -> get_path();
            update();
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
        
        auto error_dialog = Gtk::AlertDialog::create();
        error_dialog->set_message("Error selecting folder!");
        error_dialog->set_detail("There was a problem selecting the folder:\n\n" + Glib::ustring(error.what()));
        error_dialog->set_buttons({"OK"});
        error_dialog->set_cancel_button(0);
        
        error_dialog->show(*dynamic_cast<Gtk::Window*>(get_root()));
    }
}

void VideoSettings_VBox::on_select_flowbox(Gtk::FlowBoxChild * child)
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

    update();
}

void VideoSettings_VBox::on_select_row(Gtk::ListBoxRow * selected_row)
{
    // Režim
    if (selected_row -> is_ancestor(mode_listbox))
    {
        auto row0 = mode_listbox.get_row_at_index(0);
    
        if (selected_row == row0)
        {
            archive_mode_radio_button.set_active();
            target_size_hbox.set_sensitive(false);
            res_hbox.set_sensitive(false);
            fps_hbox.set_sensitive(false);
        }
        else
        {
            compress_mode_radio_button.set_active();
            target_size_hbox.set_sensitive();
            res_hbox.set_sensitive();
            fps_hbox.set_sensitive();
        }
    }

    if (selected_row -> is_ancestor(cut_listbox))
    {
        if (selected_row == cut_listbox.get_row_at_index(0))
        {
            if (cut_switch.get_active())
            {
                cut_switch.set_active(false);
                cut_start_box.set_sensitive(false);
                cut_stop_box.set_sensitive(false);
            }
            else
            {
                cut_switch.set_active();
                cut_start_box.set_sensitive();
                cut_stop_box.set_sensitive();
            }

            cut_listbox.set_selection_mode(Gtk::SelectionMode::NONE);
            cut_listbox.set_selection_mode(Gtk::SelectionMode::SINGLE);
            cut_listbox.select_row(*cut_listbox.get_row_at_index(2));
        }
    }

    update();
}

void VideoSettings_VBox::save_options(VideoElement * element)
{
    Video * video = &(element -> video);

    // Režim
    if (compress_mode_radio_button.get_active())
        {
            video -> set_compress(true);
            video -> set_bitrate_by_size(target_size_field.get_value());

            // Rozlišení a fps
            video -> set_downscale_factor(res_field.get_value());
            video -> set_output_framerate(fps_field.get_value());
        }
    else
        {
            video -> set_compress(false);
        }
       
    // Kodek
    if (codec_av1_toggle.get_active())
    {
        video -> set_codec(AV1);
        switch_codec_page(AV1);
    }
    else if (codec_hevc_toggle.get_active())
    {
        video -> set_codec(HEVC);
        switch_codec_page(HEVC);
    }
    else if (codec_vp9_toggle.get_active())
    {
        video -> set_codec(VP9);
        switch_codec_page(VP9);
    }
    
    if (!batch_settings)
    {
        // Střih
        if (cut_switch.get_active())
        {
            video -> enable_cut(true);
            float duration = video -> get_video_info().duration; 
    
            float start_secs = cut_start_s.get_value();
            float stop_secs = cut_stop_s.get_value();
    
            start_secs += cut_start_m.get_value() * 60;
            start_secs += cut_start_h.get_value() * 60 * 60;
            stop_secs += cut_stop_m.get_value() * 60;
            stop_secs += cut_stop_h.get_value() * 60 * 60;
    
            if (start_secs >= stop_secs || start_secs >= duration || stop_secs > duration)
            {
                load_options_into_GUI(video);
            }
            else
            {
                video -> set_cut(start_secs, stop_secs);
            }
        }
        else
        {
            video -> enable_cut(false);
        }
    }
    
    // Výstup a prefix
    video -> set_output_path(output_path);
    video -> set_prefix(set_prefix_field.get_text());
    output_caption.set_text("Video(s) will be saved to: \n" + video -> get_output_path());

    element -> update_labels();
}

void VideoSettings_VBox::update()
{
    // Nesmí se ukládat, když se načítá
    if (is_loading)
    {
        return;
    }

    if (batch_settings)
    {
        for (VideoElement * element : video_queue)
        {
            save_options(element);
        }
    }
    else
    {
        save_options(video_element);
    }
}

void VideoSettings_VBox::load_options_into_GUI(Video * video)
{
    is_loading = true;
    Codec codec = video -> get_codec();

    // Režim
    if (video -> is_compress_enabled())
        {
            compress_mode_radio_button.set_active();
            mode_listbox.select_row(*mode_listbox.get_row_at_index(1));
            target_size_field.set_value(video -> get_target_size());
            target_size_hbox.set_sensitive();
            res_hbox.set_sensitive();
            fps_hbox.set_sensitive();
        }
    else 
        {
            archive_mode_radio_button.set_active();
            mode_listbox.select_row(*mode_listbox.get_row_at_index(0));
            target_size_hbox.set_sensitive(false);
            res_hbox.set_sensitive(false);
            fps_hbox.set_sensitive(false);
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

    int secs = video -> get_video_info().duration;
    int mins = secs / 60;
    int hrs = mins / 60;
    mins = mins % 60;
    secs = secs % 60;
    
    if (!batch_settings)
    {
        // Střih
        if (video -> is_cutting_enabled())
        {
            int start_secs = video -> get_cut_info().startTime;
            int start_mins = start_secs / 60;
            int start_hrs = start_mins / 60;
            start_mins = start_mins % 60;
            start_secs = start_secs % 60;
    
            int stop_secs = video -> get_cut_info().endTime;
            int stop_mins = stop_secs / 60;
            int stop_hrs = stop_mins / 60;
            stop_mins = stop_mins % 60;
            stop_secs = stop_secs % 60;
    
            lim_start_h -> configure(start_hrs, 0, hrs, 1, 10, 0);
            lim_start_m -> configure(start_mins, 0, mins, 1, 10, 0);
            lim_start_s -> configure(start_secs, 0, secs, 1, 10, 0);
            lim_stop_h -> configure(stop_hrs, 0, hrs, 1, 10, 0);
            lim_stop_m -> configure(stop_mins, 0, mins, 1, 10, 0);
            lim_stop_s -> configure(stop_secs, 0, secs, 1, 10, 0);
    
            cut_start_h.set_adjustment(lim_start_h);
            cut_start_m.set_adjustment(lim_start_m);
            cut_start_s.set_adjustment(lim_start_s);
            cut_stop_h.set_adjustment(lim_stop_h);
            cut_stop_m.set_adjustment(lim_stop_m);
            cut_stop_s.set_adjustment(lim_stop_s);
    
            cut_switch.set_active();
            cut_start_box.set_sensitive();
            cut_stop_box.set_sensitive();
        }
        else
        {
            lim_start_h -> configure(0, 0, hrs, 1, 10, 0);
            lim_start_m -> configure(0, 0, mins, 1, 10, 0);
            lim_start_s -> configure(0, 0, secs, 1, 10, 0);
            lim_stop_h -> configure(hrs, 0, hrs, 1, 10, 0);
            lim_stop_m -> configure(mins, 0, mins, 1, 10, 0);
            lim_stop_s -> configure(secs, 0, secs, 1, 10, 0);
    
            cut_start_h.set_adjustment(lim_start_h);
            cut_start_m.set_adjustment(lim_start_m);
            cut_start_s.set_adjustment(lim_start_s);
            cut_stop_h.set_adjustment(lim_stop_h);
            cut_stop_m.set_adjustment(lim_stop_m);
            cut_stop_s.set_adjustment(lim_stop_s);
    
            cut_switch.set_active(false);
            cut_start_box.set_sensitive(false);
            cut_stop_box.set_sensitive(false);
        }

        // Rozlišení a fps
        res_field.set_value(video -> get_downscale_factor());
        fps_field.set_range(0, video -> get_video_info().framerate);
        fps_field.set_value(video -> get_output_framerate());
    }

    // Výstup a prefix
    set_prefix_field.set_text(video -> get_prefix());
    output_caption.set_text("Video(s) will be saved to: \n" + video -> get_output_path());

    is_loading = false;
    set_sensitive();
}

void VideoSettings_VBox::read_video_vector_options(std::vector<VideoElement *> video_vector)
{
    this -> video_queue = video_vector ;
    batch_settings = true;
    cut_listbox.set_sensitive(false);
    cut_heading.add_css_class("warning");
    cut_heading.set_text("Cut Feature (disable Select All)");

    if (video_queue.size() > 0)
    {
        Video * video = &(video_queue.at(0) -> video);
        output_path = video -> get_output_path();
        output_path = filesystem::path(output_path).parent_path().generic_string().substr(0, output_path.find("encoded_videos/"));
        load_options_into_GUI(video);
    }
}

void VideoSettings_VBox::read_video_options(VideoElement * video_element)
{
    this -> video_element = video_element;
    Video * video = &(video_element -> video);
    output_path = video -> get_output_path();
    output_path = filesystem::path(output_path).parent_path().generic_string().substr(0, output_path.find("encoded_videos/"));
    batch_settings = false;

    cut_listbox.set_sensitive();
    cut_heading.remove_css_class("warning");
    cut_heading.set_text("Cut Feature");

    load_options_into_GUI(video);
}

void VideoSettings_VBox::no_video_selected()
{
    set_sensitive(false);
}