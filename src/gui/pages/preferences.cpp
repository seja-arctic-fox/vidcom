#include "../headers/gui.h"
#include "adwaita.h"
#include "glib-object.h"
#include "glibmm/ustring.h"
#include "gtk/gtk.h"
#include "gtkmm/enums.h"
#include "sigc++/functors/mem_fun.h"
#include "src/video/video.h"
#include <iostream>

PreferencesWindow::PreferencesWindow(MainWindow * root)
:
    root(root),
    default_mode_box(Gtk::Orientation::VERTICAL),
    default_output_box(Gtk::Orientation::VERTICAL),
    reset_defaults_box(Gtk::Orientation::VERTICAL),
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
    ), 
    reset_defaults_button("Restore original defaults")
{
    dialog = ADW_PREFERENCES_DIALOG(adw_preferences_dialog_new());
    // Preferences for default values
    // -----------------
    // Group for mode
    default_mode_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    default_mode_option.append(archive_mode);
    default_mode_option.append(compress_mode);
    archive_mode.set_group(compress_mode);
    default_mode_box.append(default_mode_h);
    default_mode_box.append(default_mode_option);
    adw_preferences_group_add(
        default_mode_group, GTK_WIDGET(default_mode_box.gobj())
    );
    
    // Connect mode with GSettings
    compress_mode.set_state(SETTINGS -> get_boolean("encoding-mode"));
    archive_mode.set_state(!SETTINGS -> get_boolean("encoding-mode"));
    archive_mode.signal_toggled.connect(sigc::mem_fun(
        *this, &PreferencesWindow::set_default_mode
    ));
    compress_mode.signal_toggled.connect(sigc::mem_fun(
        *this, &PreferencesWindow::set_default_mode
    ));
    
    // Group for output
    default_output.set_button_text("Set default output folder");
    default_output_option.append(default_output);
    default_output_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    default_output_box.append(default_output_h);
    default_output_box.append(default_output_option);
    adw_preferences_group_add(
        default_output_group, GTK_WIDGET(default_output_box.gobj())
    );
    
    // Connect default output with GSettings
    string output_path = SETTINGS -> get_string("output-path");
    
    if (output_path == "")
        default_output.set_caption(
            "Video(s) will be saved next to the original file(s)."
        );
    else
        default_output.set_caption(
            "Video(s) will be saved to a subfolder in:\n" + output_path
        );
    default_output.signal_clicked.connect(sigc::mem_fun(
        *this, &PreferencesWindow::set_default_output
    ));
    
    // Group for resetting defaults
    reset_defaults_button.add_css_class("pill");
    reset_defaults_button.add_css_class("destructive-action");
    reset_defaults_box.append(reset_defaults_button);
    reset_defaults_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    adw_preferences_group_add(
        reset_defaults_group, 
        GTK_WIDGET(reset_defaults_box.gobj())
    );
    
    // Connect button to the resetting method
    reset_defaults_button.signal_clicked().connect(sigc::mem_fun(
        *this, &PreferencesWindow::reset_defaults_dialog
    ));
    
    // Setup defaults page
    defaults_page = ADW_PREFERENCES_PAGE(adw_preferences_page_new());
    adw_preferences_page_set_title(defaults_page, "Defaults");
    adw_preferences_page_set_icon_name(
        defaults_page, "org.gnome.Settings-symbolic"
    );
    adw_preferences_page_add(defaults_page, default_mode_group);
    adw_preferences_page_add(defaults_page, default_output_group);
    adw_preferences_page_add(defaults_page, reset_defaults_group);
    
    // Add page(s)
    adw_preferences_dialog_add(dialog, defaults_page);
}

PreferencesWindow::~PreferencesWindow() {}

void PreferencesWindow::set_default_mode()
{ SETTINGS -> set_boolean("encoding-mode", compress_mode.get_state()); }

void PreferencesWindow::set_default_output()
{
    string output_path = SETTINGS -> get_string("output-path");
    auto folder_picker = Gtk::FileDialog::create();
    folder_picker -> set_title("Select Output Folder");
    folder_picker -> set_modal();
    if (output_path == "")
        output_path = fs::path(getenv("HOME"));
    auto current_folder = Gio::File::create_for_path(output_path);
    folder_picker -> set_initial_folder(current_folder);
    folder_picker -> select_folder(*root, sigc::bind(sigc::mem_fun(
        *this, 
        &PreferencesWindow::on_folder_selected
    ), folder_picker));
}

void PreferencesWindow::on_folder_selected(
    Glib::RefPtr<Gio::AsyncResult> &result, 
    Glib::RefPtr<Gtk::FileDialog> folder_picker
)
{
    string output_path;
    
    try
    {
        auto folder = folder_picker -> select_folder_finish(result);

        if (folder)
        {
            output_path = folder -> get_path();
            SETTINGS -> set_string("output-path", output_path);
            default_output.set_caption(
                "Video(s) will be saved to a subfolder in:\n" + output_path
            );
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
        root -> show_toast("Error selecting folder!");
    }
}

void PreferencesWindow::reset_defaults_dialog()
{
    AdwAlertDialog * confirmation = ADW_ALERT_DIALOG(adw_alert_dialog_new(
        "Restore all defaults?", 
        "This will set all default values to the state in which they were " "right after installation. Continue?"
    ));
    adw_alert_dialog_add_responses(
        confirmation, 
        "cancel", "Cancel", 
        "restore", "Restore", 
        nullptr
    );
    adw_alert_dialog_set_response_appearance(
        confirmation, 
        "restore", ADW_RESPONSE_DESTRUCTIVE
    );
    adw_alert_dialog_set_default_response(confirmation, "cancel");
    adw_alert_dialog_set_close_response(confirmation, "cancel");
    g_signal_connect(
        confirmation, 
        "response", 
        G_CALLBACK(+[](GtkWidget *, gchar* response, gpointer data)
            { 
                string resp = response;
                if (resp == "restore")
                    static_cast<PreferencesWindow *>(data) -> reset_defaults();
            })
        , this);
    adw_dialog_present(ADW_DIALOG(confirmation), GTK_WIDGET(this -> dialog));
}

void PreferencesWindow::reset_defaults()
{
    SETTINGS -> reset("encoding-mode");
    SETTINGS -> reset("output-path");
    adw_dialog_close(ADW_DIALOG(dialog));
}
