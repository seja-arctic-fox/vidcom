#include "../headers/gui.h"
#include "adwaita.h"
#include "gio/gsettingsschema.h"
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

DefaultsPage::DefaultsPage(DummyVideoElement * video)
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
    window_content.set_margin_top(0);
    
    // Prepend title explaining the section
    defaults_desc.set_text(
        "You can set default values for all listed parameters on this page.\n"
        "Don't forget to apply the preferences by the 'Apply defaults' "
        "button below.\n"
        "You can also revert the preferences to their original state by the "
        "'Restore original defaults' button. "
    );
    defaults_desc.set_wrap();
    defaults_desc.set_justify(Gtk::Justification::FILL);
    defaults_desc_row.append(defaults_desc);
    defaults_desc_row.set_row_active(0, false);
    window_content.prepend(defaults_desc_row);
    
    // Remove unused settings from the view
    cut_heading.set_visible(false);
    cut_listbox.set_visible(false);
    fps_row.set_visible(false);
    
    // Enable rows that can be disabled by SettingsPage constructor
    target_size_row.set_sensitive();
    res_row.set_sensitive();
    
    // Extra switch for deciding whether to store next to the original video
    bool next_to_original_state = SETTINGS -> get_boolean(
        "output-next-to-original"
    );
    next_to_original_switch.set_state(next_to_original_state);
    output_row.set_sensitive(!next_to_original_state);
    output_listbox.insert(next_to_original_switch, 1);
    next_to_original_switch.signal_toggled.connect(sigc::mem_fun(
        *this, &DefaultsPage::on_nto_switched
    ));
}

DefaultsPage::~DefaultsPage()
{}

bool DefaultsPage::get_nto_state()
{ return next_to_original_switch.get_state(); }

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
    
    // Connect buttons to their methods
    apply_defaults_button.signal_clicked().connect(sigc::mem_fun(
        *this, &PreferencesWindow::apply_defaults
    ));
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

void PreferencesWindow::apply_defaults()
{
    Video * video = &dummy.video;
    
    // Basic parameters
    SETTINGS -> set_boolean(
        "encoding-mode", video -> is_compress_enabled()
    );
    SETTINGS -> set_double(
        "target-size", video -> get_target_size()
    );
    SETTINGS -> set_double(
        "downscale-factor", video -> get_downscale_factor()
    );
    SETTINGS -> set_boolean(
        "output-next-to-original", defaults_box.get_nto_state()
    );
    SETTINGS -> set_string(
        "prefix", video -> get_prefix()
    );
    SETTINGS -> set_string(
        "output-path", 
        fs::path(video -> get_output_path())
            .parent_path().parent_path().string()
    );
    SETTINGS -> set_enum(
        "codec", video -> get_codec()
    );
    
    // AV1 parameters
    SETTINGS -> set_int(
        "av1-preset", video -> AV1_options.preset
    );
    SETTINGS -> set_int(
        "av1-crf", video -> AV1_options.crf
    );
    SETTINGS -> set_boolean(
        "av1-fgs", video -> AV1_options.film_grain_synthesis
    );
    SETTINGS -> set_int(
        "av1-fgl", video -> AV1_options.film_grain_level
    );
    SETTINGS -> set_boolean(
        "av1-bd", video -> AV1_options.better_details
    );
    SETTINGS -> set_boolean(
        "av1-psy", video -> AV1_options.psychovisual_tuning
    );
    SETTINGS -> set_boolean(
        "av1-vb", video -> AV1_options.variance_boost
    );
    
    // HEVC parameters
    SETTINGS -> set_int(
        "hevc-preset", video -> HEVC_options.preset
    );
    SETTINGS -> set_int(
        "hevc-crf", video -> HEVC_options.crf
    );
    SETTINGS -> set_boolean(
        "hevc-me", video -> HEVC_options.motion_estimation
    );
    SETTINGS -> set_boolean(
        "hevc-psy", video -> HEVC_options.psychovisual_tuning
    );
    SETTINGS -> set_boolean(
        "hevc-aq", video -> HEVC_options.adaptive_quantisation
    );
    SETTINGS -> set_boolean(
        "hevc-ab", video -> HEVC_options.adaptive_b_frames
    );
    
    // VP9 parameters
    SETTINGS -> set_int(
        "vp9-preset", video -> VP9_options.preset
    );
    SETTINGS -> set_int(
        "vp9-crf", video -> VP9_options.crf
    );
    SETTINGS -> set_int(
        "vp9-cpu", video -> VP9_options.cpu_used
    );
    SETTINGS -> set_int(
        "vp9-ns", video -> VP9_options.noise_sensitivity
    );
    SETTINGS -> set_int(
        "vp9-qs", video -> VP9_options.quality
    );
    SETTINGS -> set_int(
        "vp9-tune", video -> VP9_options.tune_content
    );
    
    // AVC parameters
    SETTINGS -> set_int(
        "avc-preset", video -> AVC_options.preset
    );
    SETTINGS -> set_int(
        "avc-crf", video -> AVC_options.crf
    );
    SETTINGS -> set_boolean(
        "avc-me", video -> AVC_options.motion_estimation
    );
    SETTINGS -> set_boolean(
        "avc-aq", video -> AVC_options.adaptive_quantisation
    );
    SETTINGS -> set_boolean(
        "avc-ab", video -> AVC_options.adaptive_b_frames
    );
    SETTINGS -> set_int(
        "avc-tune", video -> AVC_options.tune
    );
    
    adw_dialog_close(ADW_DIALOG(dialog));
    root -> show_toast("New defaults have been applied");
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
    GSettingsSchema * schema = nullptr;
    g_object_get(SETTINGS -> gobj(), "settings-schema", &schema, NULL);
    gchar ** keys = g_settings_schema_list_keys(schema);
    for (gchar ** p = keys; *p != nullptr; p++)
    {
        string key = *p;
        SETTINGS -> reset(key);
    }
    g_strfreev(keys);
    g_settings_schema_unref(schema);
    
    root -> show_toast("Default values were restored to their original ones");
    adw_dialog_close(ADW_DIALOG(dialog));
}
