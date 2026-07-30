#include "../headers/gui.h"
#include "adwaita.h"
#include "gtk/gtk.h"
#include "gtkmm/enums.h"

PreferencesWindow::PreferencesWindow()
:
    default_mode_box(Gtk::Orientation::VERTICAL),
    default_output_box(Gtk::Orientation::VERTICAL),
    default_mode_h("Default Encoding Mode"),
    default_output_h("Default Output Folder"),
    archive_mode(
        "Archive", 
        "Makes the video a small as possible without loosing quality. "
    ),
    compress_mode(
        "Compress", 
        "Compresses the video to a target size. "
    ),
    default_output(
        "Saving Destination", 
        "Video(s) will be saved to: \n"
    )
{
    dialog = ADW_PREFERENCES_DIALOG(adw_preferences_dialog_new());
    
    // Page for defaults
    // 
    // Group for mode
    default_mode_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    default_mode_option.append(archive_mode);
    default_mode_option.append(compress_mode);
    archive_mode.set_group(compress_mode);
    compress_mode.set_state(SETTINGS -> get_boolean("encoding-mode"));
    archive_mode.set_state(!SETTINGS -> get_boolean("encoding-mode"));
    archive_mode.signal_toggled.connect([this]() { SETTINGS -> set_boolean("encoding-mode", compress_mode.get_state()); });
    compress_mode.signal_toggled.connect([this]() { SETTINGS -> set_boolean("encoding-mode", compress_mode.get_state()); });
    
    default_output.set_button_text("Set default output folder");
    default_output_option.append(default_output);
    default_mode_box.append(default_mode_h);
    default_mode_box.append(default_mode_option);
    adw_preferences_group_add(
        default_mode_group, GTK_WIDGET(default_mode_box.gobj())
    );
    
    // Group for output
    default_output_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    default_output_box.append(default_output_h);
    default_output_box.append(default_output_option);
    adw_preferences_group_add(
        default_output_group, GTK_WIDGET(default_output_box.gobj())
    );
    
    AdwPreferencesPage * defaults_page = 
        ADW_PREFERENCES_PAGE(adw_preferences_page_new());
    adw_preferences_page_set_title(defaults_page, "Default values");
    adw_preferences_page_add(defaults_page, default_mode_group);
    adw_preferences_page_add(defaults_page, default_output_group);
    
    // Add page(s)
    adw_preferences_dialog_add(dialog, defaults_page);
}

PreferencesWindow::~PreferencesWindow() {}
