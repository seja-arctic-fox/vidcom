#include "gtkmm/button.h"
#include "gtkmm/enums.h"
#include "../headers/gui.h"
#include "../../cli/cli.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "gtkmm/popover.h"
#include "gtkmm/scrolledwindow.h"
#include <iostream>

ResultRow::ResultRow(fs::path video_path, int status, string error_msg)
:   
    video_path(video_path),
    status(status),
    error_msg(error_msg),
    video_name(video_path.filename().string()),
    output_folder_button("Show output folder"),
    box_right(Gtk::Orientation::HORIZONTAL)
{
    video_name.add_css_class("heading");
    video_name.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
    video_name.set_margin_start(10);
    video_name.set_margin_end(10);
    
    status_icon.set_margin(10);
    
    output_folder_button.add_css_class("suggested-action");
    output_folder_button.set_margin_start(10);
    output_folder_button.set_margin_end(10);
    
    box_right.set_margin(5);
    box_right.set_expand();
    box_right.set_halign(Gtk::Align::END);
    box_right.append(status_icon);
    box_right.append(status_text);
    box_right.append(output_folder_button);
    
    set_status();
    set_output_folder_button();
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_margin(5);

    append(video_name);
    append(box_right);
}

ResultRow::~ResultRow()
{}

void ResultRow::set_status()
{
    switch(status)
    {
        case 0:
            status_icon.add_css_class("success");
            status_text.add_css_class("success");
    
            status_icon.set_from_icon_name("object-select-symbolic");
            status_text.set_markup("<b>Succesfull</b>");
            
            break;
            
        case -2:
            status_icon.add_css_class("warning");
            status_text.add_css_class("warning");
    
            status_icon.set_from_icon_name("dialog-warning-symbolic");
            status_text.set_markup("<b>Cancelled</b>");
            
            break;
            
        case -1:
            status_icon.add_css_class("error");
            status_text.add_css_class("error");
    
            status_icon.set_from_icon_name("dialog-warning-symbolic");
            status_text.set_markup("<b>Unexpected error</b>");
            
            break;
            
        case -3:
            status_icon.add_css_class("error");
            status_text.add_css_class("error");
    
            status_icon.set_from_icon_name("process-stop-symbolic");
            status_text.set_markup("<b>Incorrect output path</b>");
            
            break;
            
        default:
            status_icon.add_css_class("error");
            status_text.add_css_class("error");
            status_icon.set_from_icon_name("process-stop-symbolic");
            status_text.set_markup("<b>Error: Code " + to_string(status) + "</b>");
            
            auto error_trigger = Gtk::make_managed<Gtk::MenuButton>();
            auto error_popover = std::make_shared<Gtk::Popover>();
            auto error_label = Gtk::make_managed<Gtk::Label>();
            auto error_scrolled = Gtk::make_managed<Gtk::ScrolledWindow>();
            error_scrolled -> set_size_request(960, 540);
            error_label -> set_text(error_msg);
            error_label -> add_css_class("monospace");
            error_scrolled -> set_child(*error_label);
            error_popover -> set_child(*error_scrolled);
            error_trigger -> set_icon_name("help-about-symbolic");
            error_trigger -> add_css_class("flat");
            error_trigger -> set_popover(*error_popover);
            box_right.prepend(*error_trigger);
            break;
    }
}

void ResultRow::set_output_folder_button()
{
    if (status == -3) output_folder_button.set_sensitive(false);
    
    output_folder_button.signal_clicked().connect(([this]() 
        {
            try
            {
                auto file = Gio::File::create_for_path(filesystem::path(video_path).parent_path().generic_string());
                Gio::AppInfo::launch_default_for_uri(file -> get_uri());
            }
            catch (const Glib::Error& ex)
            {
                std::cerr << RED << "Error opening folder (" << ex.what() << ")" << RESET << std::endl;
                dynamic_cast<MainWindow *>(get_root()) -> show_toast("Error: Cannot open folder!");
            }
        }
    ));
}

void ResultRow::open_video()
{
    try
    {
        auto file = Gio::File::create_for_path(video_path);
        Gio::AppInfo::launch_default_for_uri(file -> get_uri());
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << RED << "Error opening file (" << ex.what() << ")" << RESET << std::endl;
        dynamic_cast<MainWindow *>(get_root()) -> show_toast("Error: Cannot open video!");
    }
}
