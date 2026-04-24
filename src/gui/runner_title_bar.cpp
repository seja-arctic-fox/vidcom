#include "gtkmm/enums.h"
#include "gui.h"
#include "pangomm/layout.h"
#include "sigc++/functors/mem_fun.h"
#include <string>

RunnerPanel::RunnerPanel()
:   EncodingProgressBar(),
    EncodingButton(),
    WindowTitle("No video selected")
{
    // Vlastnosti panelu
    set_expand(true);
    WindowTitle.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
    WindowTitle.add_css_class("heading");
    WindowTitle.set_margin_start(20);
    WindowTitle.set_margin_end(20);
    WindowTitle.set_max_width_chars(20);
    set_title_widget(WindowTitle);
    
    // Skrytí tlačítek, když jsou tlačítka nalevo
    // Automaticky se mi to zatím nepovedlo :(
    // Ale běžně to lidi nemění za běhu, tak to stačí
    gchar * layout = nullptr;
    g_object_get(
        gtk_settings_get_default(),
        "gtk-decoration-layout",
        &layout,
        nullptr
    );
    
    if (layout)
    {
        std::string s(layout);
        
        if (s.substr(0, s.find(":")) == "close")
            set_show_title_buttons(false);
    }
    g_free(layout);

    // Tlačítko pro zobrazení/skrytí fronty
    queue_display_button.set_icon_name("sidebar-show-symbolic");
    queue_display_button.signal_clicked().connect([this]()
        { signal_toggle_queue.emit(); });
    
    // Stavový text a ikona
    EncodingIconStatus.set_from_icon_name("tool-select-ellipse-symbolic");
    EncodingIconStatus.set_margin_start(10);
    EncodingTextStatus.set_expand(false);

    // Postup
    EncodingProgressBar.add_css_class("chunky-progress");
    EncodingProgressBar.set_expand(false);
    EncodingProgressBar.set_size_request(200);
    EncodingProgressBar.set_show_text(false);
    EncodingProgressBar.set_valign(Gtk::Align::CENTER);
    EncodingProgressBar.set_text("Nothing to do...");
    EncodingProgressBar.add_css_class("monospace");
    EncodingProgressBar.set_fraction(0);
    EncodingProgressBar.set_ellipsize(Pango::EllipsizeMode::MIDDLE);

    // Spouštěcí tlačítko
    EncodingButton.set_expand(false);
    EncodingButton.set_icon_name("media-playback-start-symbolic");
    block_encoding_button(true);
    update_status("Queue Empty", "warning");
    EncodingButton.set_halign(Gtk::Align::START);

    pack_start(queue_display_button);
    pack_start(EncodingButton);
    pack_start(EncodingIconStatus);
    pack_start(EncodingTextStatus);
    pack_end(EncodingProgressBar);

    EncodingButton.signal_clicked().connect(sigc::mem_fun(*this, &RunnerPanel::on_start_stop_clicked));
}

RunnerPanel::~RunnerPanel()
{

}

void RunnerPanel::clear_title()
{
    WindowTitle.set_text("No video selected");
}

void RunnerPanel::set_title(VideoElement * video_element)
{
    WindowTitle.set_text(
        video_element -> video_info.path.filename().string()
    );
}

void RunnerPanel::set_title_multiple(std::vector<VideoElement*>)
{
    WindowTitle.set_text("Multiple videos selected");
}

void RunnerPanel::show_queue_button(bool show)
{ queue_display_button.set_visible(show); }

void RunnerPanel::update_loading_progress(int video_index, int video_count)
{
    float i = video_index;
    EncodingProgressBar.set_fraction(i / video_count);
    EncodingProgressBar.set_text("Loading videos: " + to_string(video_index) + " / " + to_string(video_count));
}

void RunnerPanel::set_loading_state(bool is_loading)
{
    if (is_loading) 
    { 
        update_status("Loading", "warning"); 
        EncodingIconStatus.set_from_icon_name("applications-system-symbolic");
        block_encoding_button();
    }
    else if (request_button_unblock)
    { 
        update_status("Ready", "success");
        block_encoding_button(false);
        EncodingIconStatus.set_from_icon_name("selection-mode-symbolic");
        EncodingProgressBar.set_text("Nothing to do...");
        request_button_unblock = false;
    }
    else
    {
        block_encoding_button();
    }
    
    EncodingProgressBar.set_fraction(0);
}

void RunnerPanel::on_start_stop_clicked()
{
    if (!isEncoding)
    {
        signal_start_encoding.emit();
    }
    else
    {
        signal_stop_encoding.emit();
    }
}

void RunnerPanel::update_encoding_progress(const EncodingProgress& progress)
{
    double fraction = (progress.current_index - 1.0) / progress.total_count;
    fraction += (progress.progress_percent / 100.0) / progress.total_count;
    if (fraction > 1) fraction = 1;
    int percentage = fraction * 100;
    
    EncodingProgressBar.set_fraction(fraction);
    EncodingProgressBar.set_text(progress.video_name + " ( " + to_string(percentage) + "% )");
    EncodingTextStatus.set_markup("<b>Encoding... " + to_string(progress.current_index) + "/" + to_string(progress.total_count) + "</b>");
}

void RunnerPanel::set_encoding_state(bool is_encoding)
{
    if (is_encoding)
    {
        isEncoding = true;
    
        EncodingButton.remove_css_class("suggested-action");
        EncodingButton.add_css_class("destructive-action");
        EncodingButton.set_icon_name("media-playback-stop-symbolic");

        EncodingIconStatus.remove_css_class("success");
        EncodingIconStatus.add_css_class("warning");
        EncodingIconStatus.set_from_icon_name("applications-system-symbolic");

        EncodingTextStatus.remove_css_class("success");
        EncodingTextStatus.add_css_class("warning");
        EncodingTextStatus.set_markup("<b>Encoding...</b>");
    }
    else
    {
        isEncoding = false;

        EncodingButton.set_icon_name("media-playback-start-symbolic");
        EncodingButton.remove_css_class("destructive-action");
        EncodingButton.add_css_class("suggested-action");

        EncodingIconStatus.remove_css_class("warning");
        EncodingIconStatus.add_css_class("success");
        EncodingIconStatus.set_from_icon_name("selection-mode-symbolic");

        EncodingTextStatus.remove_css_class("warning");
        EncodingTextStatus.add_css_class("success");
        EncodingTextStatus.set_markup("<b>Ready</b>");

        EncodingProgressBar.set_text("Nothing to do...");
        EncodingProgressBar.set_fraction(0.0);
    }
}

void RunnerPanel::update_status(const std::string& status, const std::string& css_class)
{
    EncodingTextStatus.set_markup("<b>" + status + "</b>");

    if (!css_class.empty())
    {
        EncodingIconStatus.remove_css_class("warning");
        EncodingIconStatus.remove_css_class("success");
        EncodingIconStatus.remove_css_class("error");
        EncodingIconStatus.add_css_class(css_class);

        EncodingTextStatus.remove_css_class("warning");
        EncodingTextStatus.remove_css_class("success");
        EncodingTextStatus.remove_css_class("error");
        EncodingTextStatus.add_css_class(css_class);
    }
    
    if (status == "Queue Empty")
        EncodingIconStatus.set_from_icon_name("tool-select-ellipse-symbolic");
}

void RunnerPanel::block_encoding_button(bool block)
{
    EncodingButton.set_sensitive(!block);
    
    if (block)
    {
        EncodingButton.remove_css_class("suggested-action");
        EncodingButton.remove_css_class("destructive-action");
    }
    else
    {
        EncodingButton.add_css_class("suggested-action");
    }
}
