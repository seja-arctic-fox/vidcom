#include "gtkmm/spinbutton.h"
#include "gui.h"
#include "sigc++/functors/mem_fun.h"
#include "src/video/video.h"
#include <vector>

HEVC_Parameters::HEVC_Parameters(VideoElement * video_element)
:   video_element(video_element),
    is_loading(false),
    preset_text("Encoding Preset"),
    crf_text("CRF"),
    me_text("Motion Estimation"),
    aq_text("Adaptive Quantisation"),
    pt_text("Psychovisual Tuning"),
    ab_text("Adaptive B-Frames"),
    preset_caption("Lower value means better compression but longer encoding time. "),
    crf_caption("Quality level. Lower values increase quality and bitrate. "),
    me_caption("Enables better motion estimation. Good for better compression, but increases encoding time. "),
    aq_caption("Enables adaptive quantisation. Better quality in frames with more details. "),
    pt_caption("Enables psychovisual tuning (better for human eye). "),
    ab_caption("Enables adaptive B-frames. Inserts as much B-frames as needed for better compression. "), 
    preset_hbox(Gtk::Orientation::HORIZONTAL),
    crf_hbox(Gtk::Orientation::HORIZONTAL),
    me_hbox(Gtk::Orientation::HORIZONTAL),
    aq_hbox(Gtk::Orientation::HORIZONTAL),
    pt_hbox(Gtk::Orientation::HORIZONTAL),
    ab_hbox(Gtk::Orientation::HORIZONTAL),
    preset_vbox(Gtk::Orientation::VERTICAL),
    crf_vbox(Gtk::Orientation::VERTICAL),
    me_vbox(Gtk::Orientation::VERTICAL),
    aq_vbox(Gtk::Orientation::VERTICAL),
    pt_vbox(Gtk::Orientation::VERTICAL),
    ab_vbox(Gtk::Orientation::VERTICAL),
    preset_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(3, 0, 9))),
    crf_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(19, 0, 63)))
{
     add_css_class("navigation-sidebar");
    add_css_class("card");
    set_margin_bottom(20);

    preset_text.set_halign(Gtk::Align::START);
    crf_text.set_halign(Gtk::Align::START);
    me_text.set_halign(Gtk::Align::START);
    aq_text.set_halign(Gtk::Align::START);
    pt_text.set_halign(Gtk::Align::START);
    ab_text.set_halign(Gtk::Align::START);
    preset_caption.set_halign(Gtk::Align::START);
    crf_caption.set_halign(Gtk::Align::START);
    me_caption.set_halign(Gtk::Align::START);
    aq_caption.set_halign(Gtk::Align::START);
    pt_caption.set_halign(Gtk::Align::START);
    ab_caption.set_halign(Gtk::Align::START);

    me_w.set_can_target(false);
    me_w.set_halign(Gtk::Align::CENTER);
    me_w.set_valign(Gtk::Align::CENTER);
    aq_w.set_can_target(false);
    aq_w.set_halign(Gtk::Align::CENTER);
    aq_w.set_valign(Gtk::Align::CENTER);
    pt_w.set_can_target(false);
    pt_w.set_halign(Gtk::Align::CENTER);
    pt_w.set_valign(Gtk::Align::CENTER);
    ab_w.set_can_target(false);
    ab_w.set_halign(Gtk::Align::CENTER);
    ab_w.set_valign(Gtk::Align::CENTER);

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

    me_hbox.set_margin(5);
    me_vbox.set_margin(5);
    me_text.set_margin(2);
    me_caption.set_margin(2);
    me_caption.set_ellipsize(Pango::EllipsizeMode::END);
    me_text.add_css_class("heading");
    me_caption.add_css_class("caption");
    me_vbox.append(me_text);
    me_vbox.append(me_caption);
    me_w.set_margin(10);
    me_hbox.append(me_w);
    me_hbox.append(me_vbox);


    aq_hbox.set_margin(5);
    aq_vbox.set_margin(5);
    aq_text.set_margin(2);
    aq_caption.set_margin(2);
    aq_caption.set_ellipsize(Pango::EllipsizeMode::END);
    aq_text.add_css_class("heading");
    aq_caption.add_css_class("caption");
    aq_vbox.append(aq_text);
    aq_vbox.append(aq_caption);
    aq_w.set_margin(10);
    aq_hbox.append(aq_w);
    aq_hbox.append(aq_vbox);

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

    ab_hbox.set_margin(5);
    ab_vbox.set_margin(5);
    ab_text.set_margin(2);
    ab_caption.set_margin(2);
    ab_caption.set_ellipsize(Pango::EllipsizeMode::END);
    ab_text.add_css_class("heading");
    ab_caption.add_css_class("caption");
    ab_vbox.append(ab_text);
    ab_vbox.append(ab_caption);
    ab_w.set_margin(10);
    ab_hbox.append(ab_w);
    ab_hbox.append(ab_vbox);

    append(preset_hbox);
    append(crf_hbox);
    append(me_hbox);
    append(pt_hbox);
    append(aq_hbox);
    append(ab_hbox);

    get_row_at_index(0 )-> set_activatable(false);
    get_row_at_index(0 )-> set_selectable(false);
    get_row_at_index(1 )-> set_activatable(false);
    get_row_at_index(1 )-> set_selectable(false);

    signal_row_selected().connect(sigc::mem_fun(*this, &HEVC_Parameters::on_select_row));
    std::vector<Gtk::SpinButton *> fields = {&preset_w, &crf_w};
    for (auto f : fields)
    {
        f -> signal_value_changed().connect(sigc::mem_fun(*this, &HEVC_Parameters::update));
    }
}

HEVC_Parameters::~HEVC_Parameters()
{}

void HEVC_Parameters::on_select_row(Gtk::ListBoxRow * row)
{
    set_selection_mode(Gtk::SelectionMode::NONE);
    select_row(*get_row_at_index(0));
    set_selection_mode(Gtk::SelectionMode::SINGLE);

    if (row == get_row_at_index(2))
    {
        if (me_w.get_active())
        {
            me_w.set_active(false);
        }
        else
        {
            me_w.set_active();
        }
    }

    if (row == get_row_at_index(3))
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

    if (row == get_row_at_index(4))
    {
        if (aq_w.get_active())
        {
            aq_w.set_active(false);
        }
        else
        {
            aq_w.set_active();
        }
    }

    if (row == get_row_at_index(5))
    {
        if (ab_w.get_active())
        {
            ab_w.set_active(false);
        }
        else
        {
            ab_w.set_active();
        }
    }

    update();
}

void HEVC_Parameters::update()
{
    if (is_loading)
    {
        return;
    }

    HEVC_options * video_options = &(video_element -> video.HEVC_options);

    video_options -> preset = preset_w.get_value();
    video_options -> crf = crf_w.get_value();
    video_options -> motion_estimation = me_w.get_active();
    video_options -> psychovisual_tuning = pt_w.get_active();
    video_options -> adaptive_quantisation = aq_w.get_active();
    video_options -> adaptive_b_frames = ab_w.get_active();
}

void HEVC_Parameters::load()
{
    is_loading = true;

    HEVC_options video_options = video_element -> video.HEVC_options;

    preset_w.set_value(video_options.preset);
    crf_w.set_value(video_options.crf);
    me_w.set_active(video_options.motion_estimation);
    pt_w.set_active(video_options.psychovisual_tuning);
    aq_w.set_active(video_options.adaptive_quantisation);
    ab_w.set_active(video_options.adaptive_b_frames);

    is_loading = false;
}