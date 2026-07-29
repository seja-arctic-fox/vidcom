#include "../headers/gui.h"
#include "gtkmm/enums.h"

PreferencesWindow::PreferencesWindow()
:
    header_bar(), 
    content_box(Gtk::Orientation::VERTICAL),
    pref_title("Preferences"),
    default_mode_h("Default Encoding Mode"),
    default_output_h("Default Output Folder"),
    archive_mode("Archive", "efefefefefefeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
    compress_mode("Compress", "ofwoeijfoewjfioewjfiowejfioewjfewiofjoi"),
    default_output("Output folder", "set somethingsdfddofjksokd")
{
    pref_title.add_css_class("heading");
    header_bar.add_css_class("flat");
    header_bar.set_title_widget(pref_title);
    set_titlebar(header_bar);
    
    content_box.set_margin(50);
    content_box.set_halign(Gtk::Align::CENTER);
    
    archive_mode.set_group(compress_mode);
    default_mode_box.set_expand();
    default_mode_box.append(archive_mode);
    default_mode_box.append(compress_mode);
    default_output_box.append(default_output);
    
    content_box.append(default_mode_h);
    content_box.append(default_mode_box);
    content_box.append(default_output_h);
    content_box.append(default_output_box);
    
    scrolled.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scrolled.set_child(content_box);
    
    set_child(scrolled);
    set_size_request(300, -1);
    set_default_size(600, 540);
    set_resizable();
    set_modal();
    present();
}

PreferencesWindow::~PreferencesWindow() {}
