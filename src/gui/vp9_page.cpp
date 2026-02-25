#include "glibmm/refptr.h"
#include "gtkmm/adjustment.h"
#include "gtkmm/enums.h"
#include "gtkmm/scale.h"
#include "gui.h"
#include "sigc++/functors/mem_fun.h"

VP9_Parameters::VP9_Parameters(VideoElement * video_element)
:   video_element(video_element),
    is_loading(false),
    preset_text("Encoding Preset"),
    crf_text("CRF"),
    cpu_text("CPU Usage"),
    q_text("Quality Scale"),
    ns_text("Noise Sensitivity"),
    t_text("Tune"),
    preset_caption("Lower value means better compression but longer encoding time. "),
    crf_caption("Quality level. Lower values increase quality and bitrate. "),
    cpu_caption("Amount of CPU used during encoding. Lower values mean better compression but longer encoding time. "),
    q_caption("0 = best, 1 = realtime, 2 = good"),
    ns_caption("Sensitivity of the deblocking filter which removes encoding artifacts. "),
    t_caption("0 = default, 1 = screen recording, 2 = film content"), 
    preset_hbox(Gtk::Orientation::HORIZONTAL),
    crf_hbox(Gtk::Orientation::HORIZONTAL),
    cpu_hbox(Gtk::Orientation::HORIZONTAL),
    q_hbox(Gtk::Orientation::HORIZONTAL),
    ns_hbox(Gtk::Orientation::HORIZONTAL),
    t_hbox(Gtk::Orientation::HORIZONTAL),
    preset_vbox(Gtk::Orientation::VERTICAL),
    crf_vbox(Gtk::Orientation::VERTICAL),
    cpu_vbox(Gtk::Orientation::VERTICAL),
    q_vbox(Gtk::Orientation::VERTICAL),
    ns_vbox(Gtk::Orientation::VERTICAL),
    t_vbox(Gtk::Orientation::VERTICAL),
    cpu_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(0, -8, 8))),
    ns_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(4, 0, 4))),
    preset_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(3, 0, 9))),
    crf_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(19, 0, 63))),
    q_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(0, 0, 2))),
    t_w(Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(0, 0, 2)))
{
     add_css_class("navigation-sidebar");
    add_css_class("card");
    set_margin_bottom(20);

    preset_text.set_halign(Gtk::Align::START);
    crf_text.set_halign(Gtk::Align::START);
    cpu_text.set_halign(Gtk::Align::START);
    q_text.set_halign(Gtk::Align::START);
    ns_text.set_halign(Gtk::Align::START);
    t_text.set_halign(Gtk::Align::START);
    preset_caption.set_halign(Gtk::Align::START);
    crf_caption.set_halign(Gtk::Align::START);
    cpu_caption.set_halign(Gtk::Align::START);
    q_caption.set_halign(Gtk::Align::START);
    ns_caption.set_halign(Gtk::Align::START);
    t_caption.set_halign(Gtk::Align::START);

    q_w.set_draw_value();
    q_w.set_increments(1, 2);
    q_w.set_digits(0);
    q_w.set_size_request(110);
    t_w.set_draw_value();
    t_w.set_increments(1, 2);
    t_w.set_digits(0);
    t_w.set_size_request(110);

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

    cpu_hbox.set_margin(5);
    cpu_vbox.set_margin(5);
    cpu_text.set_margin(2);
    cpu_caption.set_margin(2);
    cpu_caption.set_ellipsize(Pango::EllipsizeMode::END);
    cpu_text.add_css_class("heading");
    cpu_caption.add_css_class("caption");
    cpu_vbox.append(cpu_text);
    cpu_vbox.append(cpu_caption);
    cpu_w.set_margin(10);
    cpu_hbox.append(cpu_w);
    cpu_hbox.append(cpu_vbox);


    q_hbox.set_margin(5);
    q_vbox.set_margin(5);
    q_text.set_margin(2);
    q_caption.set_margin(2);
    q_caption.set_ellipsize(Pango::EllipsizeMode::END);
    q_text.add_css_class("heading");
    q_caption.add_css_class("caption");
    q_vbox.append(q_text);
    q_vbox.append(q_caption);
    q_w.set_margin(10);
    q_hbox.append(q_w);
    q_hbox.append(q_vbox);

    ns_hbox.set_margin(5);
    ns_vbox.set_margin(5);
    ns_text.set_margin(2);
    ns_caption.set_margin(2);
    ns_caption.set_ellipsize(Pango::EllipsizeMode::END);
    ns_text.add_css_class("heading");
    ns_caption.add_css_class("caption");
    ns_vbox.append(ns_text);
    ns_vbox.append(ns_caption);
    ns_w.set_margin(10);
    ns_hbox.append(ns_w);
    ns_hbox.append(ns_vbox);

    t_hbox.set_margin(5);
    t_vbox.set_margin(5);
    t_text.set_margin(2);
    t_caption.set_margin(2);
    t_caption.set_ellipsize(Pango::EllipsizeMode::END);
    t_text.add_css_class("heading");
    t_caption.add_css_class("caption");
    t_vbox.append(t_text);
    t_vbox.append(t_caption);
    t_w.set_margin(10);
    t_hbox.append(t_w);
    t_hbox.append(t_vbox);

    append(preset_hbox);
    append(crf_hbox);
    append(cpu_hbox);
    append(ns_hbox);
    append(q_hbox);
    append(t_hbox);

    for (int i = 0; i < 6; i++)
    {
        get_row_at_index(i )-> set_activatable(false);
        get_row_at_index(i )-> set_selectable(false);
    }
    
    std::vector<Gtk::SpinButton *> fields = {&preset_w, &crf_w, &cpu_w, &ns_w};
    for (auto f : fields)
    {
        f -> signal_value_changed().connect(sigc::mem_fun(*this, &VP9_Parameters::update));
    }
    std::vector<Gtk::Scale *> sliders = {&q_w, &t_w};
    for (auto s : sliders)
    {
        s -> signal_change_value().connect(sigc::mem_fun(*this, &VP9_Parameters::on_move_slider), false);
    }
}

VP9_Parameters::~VP9_Parameters()
{}

bool VP9_Parameters::on_move_slider(Gtk::ScrollType, double)
{
    update();
    return false;
}

void VP9_Parameters::update()
{
    if (is_loading)
    {
        return;
    }

    VP9_options * video_options = &(video_element -> video.VP9_options);

    video_options -> preset = preset_w.get_value();
    video_options -> crf = crf_w.get_value();
    video_options -> cpu_used = cpu_w.get_value();
    video_options -> noise_sensitivity = ns_w.get_value();
    video_options -> quality = q_w.get_value();
    video_options -> tune_content = t_w.get_value();
}

void VP9_Parameters::load()
{
    is_loading = true;

    VP9_options video_options = video_element -> video.VP9_options;

    preset_w.set_value(video_options.preset);
    crf_w.set_value(video_options.crf);
    cpu_w.set_value(video_options.cpu_used);
    ns_w.set_value(video_options.noise_sensitivity);
    q_w.set_value(video_options.quality);
    t_w.set_value(video_options.tune_content);

    is_loading = false;
}