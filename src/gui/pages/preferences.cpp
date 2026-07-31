#include "../headers/gui.h"
#include "adwaita.h"
#include "glib-object.h"
#include "glibmm/ustring.h"
#include "gtk/gtk.h"
#include "gtkmm/enums.h"
#include "sigc++/functors/mem_fun.h"
#include "src/video/video.h"

DummyVideoElement::DummyVideoElement()
:   
    VideoElement("")
{}

DummyVideoElement::~DummyVideoElement()
{}

DefaultsPage::DefaultsPage(VideoElement * video)
:   
    SettingsPage(),
    next_to_original_switch(
        "Save next to original",
        "Save video(s) next to original file(s)"
    )
{
    // Load dummy video
    SettingsPage::read_video_options(video);
    set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::NEVER);
    
    // Remove unused settings from the view
    cut_heading.set_visible(false);
    cut_listbox.set_visible(false);
    fps_row.set_visible(false);
    
    // Extra switch for deciding whether to store next to the original video
    bool output_next_to_original = SETTINGS -> get_boolean(
        "output-next-to-original"
    );
    next_to_original_switch.set_state(output_next_to_original);
    output_row.set_sensitive(!output_next_to_original);
    output_listbox.insert(next_to_original_switch, 1);
    next_to_original_switch.signal_toggled.connect(sigc::mem_fun(
        *this, &DefaultsPage::on_nto_switched
    ));
}

DefaultsPage::~DefaultsPage()
{}

void DefaultsPage::on_nto_switched()
{ output_row.set_sensitive(!next_to_original_switch.get_state()); }

// Only reason for this is removing the sensitivity setters
// (they don't make sense here)
void DefaultsPage::save_archive_mode(VideoElement * element)
{ element -> video.set_compress(false); }

void DefaultsPage::save_compress_mode(VideoElement * element)
{ element -> video.set_compress(true); }

PreferencesWindow::PreferencesWindow(MainWindow * root)
:
    root(root),
    dummy(),
    defaults_box(&dummy),
    set_defaults_box(Gtk::Orientation::VERTICAL),
    apply_defaults_button("Apply defaults"),
    reset_defaults_button("Restore original defaults")
{
    dialog = ADW_PREFERENCES_DIALOG(adw_preferences_dialog_new());
    
    // Preferences for default values
    // -----------------
    defaults_box.set_size_request(root -> get_width() * 0.5, -1);
    defaults_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    adw_preferences_group_add(defaults_group, GTK_WIDGET(defaults_box.gobj()));
    
    // Group for resetting defaults
    apply_defaults_button.add_css_class("pill");
    apply_defaults_button.add_css_class("suggested-action");
    apply_defaults_button.set_margin(5);
    reset_defaults_button.add_css_class("pill");
    reset_defaults_button.add_css_class("destructive-action");
    reset_defaults_button.set_margin(5);
    set_defaults_box.append(apply_defaults_button);
    set_defaults_box.append(reset_defaults_button);
    reset_defaults_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    adw_preferences_group_add(
        reset_defaults_group, 
        GTK_WIDGET(set_defaults_box.gobj())
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
    adw_preferences_page_add(defaults_page, defaults_group);
    adw_preferences_page_add(defaults_page, reset_defaults_group);
    
    // Add page(s)
    adw_preferences_dialog_add(dialog, defaults_page);
    adw_dialog_set_content_width(ADW_DIALOG(dialog), root -> get_width() * 0.75);
    adw_dialog_set_content_height(ADW_DIALOG(dialog), root -> get_height() * 0.75);
}

PreferencesWindow::~PreferencesWindow() {}

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
