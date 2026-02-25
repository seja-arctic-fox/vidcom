#include "glibmm/refptr.h"
#include "gtkmm/adjustment.h"
#include "gtkmm/enums.h"
#include "gtkmm/listboxrow.h"
#include "gui.h"
#include "sigc++/functors/mem_fun.h"
#include "src/video/video.h"
#include <vector>

AV1_Parameters::AV1_Parameters(VideoElement * video_element)
:   video_element(video_element),
    is_loading(false),
    preset_text("Encoding Preset"),
    crf_text("CRF"),
    fgs_text("Film Grain Synthesis"),
    fgl_text("Film Grain Level"),
    bd_text("Better Details"),
    pt_text("Psychovisual Tuning"),
    vb_text("Variance Boost"),
    preset_caption("Lower value means better compression but longer encoding time. "),
    crf_caption("Quality level. Lower values increase quality and bitrate. "),
    fgs_caption("Enables film grain synthesis postprocessing filter. "),
    fgl_caption("Level of film grain in shot. "), 
    bd_caption("Enables overlay frames for better details. "),
    pt_caption("Enables psychovisual tuning (better for human eye), instead of PSNR (exact method). "),
    vb_caption("Adaptively increases bitrate when needed. Not recommended to use with COMPRESS mode. "), 
    preset_hbox(Gtk::Orientation::HORIZONTAL),
    crf_hbox(Gtk::Orientation::HORIZONTAL),
    fgs_hbox(Gtk::Orientation::HORIZONTAL),
    fgl_hbox(Gtk::Orientation::HORIZONTAL),
    bd_hbox(Gtk::Orientation::HORIZONTAL),
    pt_hbox(Gtk::Orientation::HORIZONTAL),
    vb_hbox(Gtk::Orientation::HORIZONTAL),
    preset_vbox(Gtk::Orientation::VERTICAL),
    crf_vbox(Gtk::Orientation::VERTICAL),
    fgs_vbox(Gtk::Orientation::VERTICAL),
    fgl_vbox(Gtk::Orientation::VERTICAL),
    bd_vbox(Gtk::Orientation::VERTICAL),
    pt_vbox(Gtk::Orientation::VERTICAL),
    vb_vbox(Gtk::Orientation::VERTICAL),
    preset_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(3, 0, 13))),
    crf_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(35, 0, 63))),
    fgl_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(16, 0, 32)))
{
    add_css_class("navigation-sidebar");
    add_css_class("card");
    set_margin_bottom(20);

    preset_text.set_halign(Gtk::Align::START);
    crf_text.set_halign(Gtk::Align::START);
    fgs_text.set_halign(Gtk::Align::START);
    fgl_text.set_halign(Gtk::Align::START);
    bd_text.set_halign(Gtk::Align::START);
    pt_text.set_halign(Gtk::Align::START);
    vb_text.set_halign(Gtk::Align::START);
    preset_caption.set_halign(Gtk::Align::START);
    crf_caption.set_halign(Gtk::Align::START);
    fgs_caption.set_halign(Gtk::Align::START);
    fgl_caption.set_halign(Gtk::Align::START);
    bd_caption.set_halign(Gtk::Align::START);
    pt_caption.set_halign(Gtk::Align::START);
    vb_caption.set_halign(Gtk::Align::START);

    fgs_w.set_can_target(false);
    fgs_w.set_halign(Gtk::Align::CENTER);
    fgs_w.set_valign(Gtk::Align::CENTER);
    bd_w.set_can_target(false);
    bd_w.set_halign(Gtk::Align::CENTER);
    bd_w.set_valign(Gtk::Align::CENTER);
    pt_w.set_can_target(false);
    pt_w.set_halign(Gtk::Align::CENTER);
    pt_w.set_valign(Gtk::Align::CENTER);
    vb_w.set_can_target(false);
    vb_w.set_halign(Gtk::Align::CENTER);
    vb_w.set_valign(Gtk::Align::CENTER);

    preset_hbox.set_margin(5);
    preset_vbox.set_margin(5);
    preset_text.set_margin(2);
    preset_caption.set_margin(2);
    preset_caption.set_ellipsize(Pango::EllipsizeMode::END);
    preset_text.add_css_class("heading");
    preset_caption.add_css_class("caption");
    preset_vbox.append(preset_text);
    preset_vbox.append(preset_caption);
    preset_w.set_margin(10);
    preset_hbox.append(preset_w);
    preset_hbox.append(preset_vbox);

    crf_hbox.set_margin(5);
    crf_vbox.set_margin(5);
    crf_text.set_margin(2);
    crf_caption.set_margin(2);
    crf_caption.set_ellipsize(Pango::EllipsizeMode::END);
    crf_text.add_css_class("heading");
    crf_caption.add_css_class("caption");
    crf_vbox.append(crf_text);
    crf_vbox.append(crf_caption);
    crf_w.set_margin(10);
    crf_hbox.append(crf_w);
    crf_hbox.append(crf_vbox);

    fgs_hbox.set_margin(5);
    fgs_vbox.set_margin(5);
    fgs_text.set_margin(2);
    fgs_caption.set_margin(2);
    fgs_caption.set_ellipsize(Pango::EllipsizeMode::END);
    fgs_text.add_css_class("heading");
    fgs_caption.add_css_class("caption");
    fgs_vbox.append(fgs_text);
    fgs_vbox.append(fgs_caption);
    fgs_w.set_margin(10);
    fgs_hbox.append(fgs_w);
    fgs_hbox.append(fgs_vbox);

    fgl_hbox.set_margin(5);
    fgl_vbox.set_margin(5);
    fgl_text.set_margin(2);
    fgl_caption.set_margin(2);
    fgl_caption.set_ellipsize(Pango::EllipsizeMode::END);
    fgl_text.add_css_class("heading");
    fgl_caption.add_css_class("caption");
    fgl_vbox.append(fgl_text);
    fgl_vbox.append(fgl_caption);
    fgl_w.set_margin(10);
    fgl_hbox.append(fgl_w);
    fgl_hbox.append(fgl_vbox);
    fgl_hbox.set_sensitive(false);

    bd_hbox.set_margin(5);
    bd_vbox.set_margin(5);
    bd_text.set_margin(2);
    bd_caption.set_margin(2);
    bd_caption.set_ellipsize(Pango::EllipsizeMode::END);
    bd_text.add_css_class("heading");
    bd_caption.add_css_class("caption");
    bd_vbox.append(bd_text);
    bd_vbox.append(bd_caption);
    bd_w.set_margin(10);
    bd_hbox.append(bd_w);
    bd_hbox.append(bd_vbox);

    pt_hbox.set_margin(5);
    pt_vbox.set_margin(5);
    pt_text.set_margin(2);
    pt_caption.set_margin(2);
    pt_caption.set_ellipsize(Pango::EllipsizeMode::END);
    pt_text.add_css_class("heading");
    pt_caption.add_css_class("caption");
    pt_vbox.append(pt_text);
    pt_vbox.append(pt_caption);
    pt_w.set_margin(10);
    pt_hbox.append(pt_w);
    pt_hbox.append(pt_vbox);

    vb_hbox.set_margin(5);
    vb_vbox.set_margin(5);
    vb_text.set_margin(2);
    vb_caption.set_margin(2);
    vb_caption.set_ellipsize(Pango::EllipsizeMode::END);
    vb_text.add_css_class("heading");
    vb_caption.add_css_class("caption");
    vb_vbox.append(vb_text);
    vb_vbox.append(vb_caption);
    vb_w.set_margin(10);
    vb_hbox.append(vb_w);
    vb_hbox.append(vb_vbox);

    append(preset_hbox);
    append(crf_hbox);
    append(fgs_hbox);
    append(fgl_hbox);
    append(bd_hbox);
    append(pt_hbox);
    append(vb_hbox);

    get_row_at_index(0 )-> set_activatable(false);
    get_row_at_index(0 )-> set_selectable(false);
    get_row_at_index(1 )-> set_activatable(false);
    get_row_at_index(1 )-> set_selectable(false);
    get_row_at_index(3 )-> set_activatable(false);
    get_row_at_index(3 )-> set_selectable(false);

    // Signály
    signal_row_selected().connect(sigc::mem_fun(*this, &AV1_Parameters::on_select_row));
    std::vector<Gtk::SpinButton *> fields = {&preset_w, &crf_w, &fgl_w};
    for (auto f : fields)
    {
        f -> signal_value_changed().connect(sigc::mem_fun(*this, &AV1_Parameters::update));
    }
}

AV1_Parameters::~AV1_Parameters()
{}

void AV1_Parameters::on_select_row(Gtk::ListBoxRow * row)
{
    set_selection_mode(Gtk::SelectionMode::NONE);
    select_row(*get_row_at_index(0));
    set_selection_mode(Gtk::SelectionMode::SINGLE);

    if (row == get_row_at_index(2))
    {
        if (fgs_w.get_active())
        {
            fgs_w.set_active(false);
            fgl_hbox.set_sensitive(false);
        }
        else
        {
            fgs_w.set_active();
            fgl_hbox.set_sensitive();
        }
    }

    if (row == get_row_at_index(4))
    {
        if (bd_w.get_active())
        {
            bd_w.set_active(false);
        }
        else
        {
            bd_w.set_active();
        }
    }

    if (row == get_row_at_index(5))
    {
        if (pt_w.get_active())
        {
            pt_w.set_active(false);
        }
        else
        {
            pt_w.set_active();
        }
    }

    if (row == get_row_at_index(6))
    {
        if (vb_w.get_active())
        {
            vb_w.set_active(false);
        }
        else
        {
            vb_w.set_active();
        }
    }

    update();
}

void AV1_Parameters::update()
{
    if (is_loading)
    {
        return;
    }

    AV1_options * video_options = &(video_element -> video.AV1_options);

    video_options -> preset = preset_w.get_value();
    video_options -> crf = crf_w.get_value();
    video_options -> film_grain_synthesis = fgs_w.get_active();
    video_options -> film_grain_level = fgl_w.get_value();
    video_options -> better_details = bd_w.get_active();
    video_options -> psychovisual_tuning = pt_w.get_active();
    video_options -> variance_boost = vb_w.get_active();
}

void AV1_Parameters::load()
{
    is_loading = true;

    AV1_options video_options = video_element -> video.AV1_options;

    preset_w.set_value(video_options.preset);
    crf_w.set_value(video_options.crf);
    fgs_w.set_active(video_options.film_grain_synthesis);

    if (video_options.film_grain_synthesis)
    {
        fgl_hbox.set_sensitive();
    }
    else
    {
        fgl_hbox.set_sensitive(false);
    }

    bd_w.set_active(video_options.better_details);
    pt_w.set_active(video_options.psychovisual_tuning);
    vb_w.set_active(video_options.variance_boost);

    is_loading = false;
}