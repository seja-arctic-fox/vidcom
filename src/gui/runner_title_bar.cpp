#include "gui.h"
#include "pangomm/layout.h"
#include "sigc++/functors/mem_fun.h"
#include <string>

RunnerPanel::RunnerPanel()
:   isEncoding(false),
    EncodingProgressBar(),
    EncodingButton(),
    WindowTitle("VidCom GUI 0.8")
{
    // Vlastnosti panelu
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_margin(5);
    set_margin_top(10);
    set_spacing(10);
    set_expand(true);
    set_margin_end(20);
    WindowTitle.add_css_class("heading");

    // Stavový text a ikona
    EncodingTextStatus.add_css_class("success");
    EncodingIconStatus.add_css_class("success");
    EncodingIconStatus.set_from_icon_name("selection-mode-symbolic");
    EncodingTextStatus.set_markup("<b>Ready</b>");
    EncodingTextStatus.set_expand(false);

    // Postup
    EncodingProgressBar.add_css_class("chunky-progress");
    EncodingProgressBar.set_expand(true);
    EncodingProgressBar.set_show_text();
    EncodingProgressBar.set_text("Nothing to do...");
    EncodingProgressBar.add_css_class("monospace");
    EncodingProgressBar.set_fraction(0);
    EncodingProgressBar.set_ellipsize(Pango::EllipsizeMode::MIDDLE);

    // Spouštěcí tlačítko
    EncodingButton.set_expand(false);
    EncodingButton.set_icon_name("media-playback-start-symbolic");
    EncodingButton.add_css_class("suggested-action");
    EncodingButton.add_css_class("pill");
    EncodingButton.set_halign(Gtk::Align::START);

    append(WindowTitle);
    append(EncodingButton);
    append(EncodingIconStatus);
    append(EncodingTextStatus);
    append(EncodingProgressBar);

    EncodingButton.signal_clicked().connect(sigc::mem_fun(*this, &RunnerPanel::on_start_stop_clicked));
}

RunnerPanel::~RunnerPanel()
{

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

void RunnerPanel::update_progress(const EncodingProgress& progress)
{
    EncodingProgressBar.set_fraction(progress.progress_percent / 100.0);
    EncodingProgressBar.set_text(progress.video_name + " ( " + to_string(progress.progress_percent) + "% )");
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
}

void RunnerPanel::block_encoding_button(bool block)
{
    EncodingButton.set_sensitive(!block);
}