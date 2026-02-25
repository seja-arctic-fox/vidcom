#include "gdkmm/cursor.h"
#include "gdkmm/enums.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtkmm/dragsource.h"
#include "gtkmm/droptarget.h"
#include "gtkmm/enums.h"
#include "gtkmm/filechooser.h"
#include "gtkmm/listbox.h"
#include "gtkmm/listboxrow.h"
#include "gtkmm/widgetpaintable.h"
#include "gui.h"
#include "pangomm/layout.h"
#include "sigc++/functors/mem_fun.h"
#include "src/video/video.h"
#include <filesystem>
#include <iostream>
#include <string>

// Pomocná funkce pro formátování času
string format_time(float video_duration_s)
{
    int sec = video_duration_s;
    int min = sec / 60;
    int hour = min / 60;
    min = min % 60;
    sec = sec % 60;

    string st_sec = to_string(sec);
    string st_min = to_string(min);
    string st_hour = to_string(hour);

    if (st_sec.length() == 1)
    {
        st_sec = "0" + st_sec;
    }

    if (st_min.length() == 1)
    {
        st_min = "0" + st_min;
    }

    if (hour == 0)
    {
        return st_min + ":" + st_sec + " s";
    }
    else
    {
        if (st_hour.length() == 1)
        {
            st_hour = "0" + st_hour;
        }
    
        return st_hour + ":" + st_min + ":" + st_sec + " s";
    }

}

VideoElement::VideoElement(std::string input_path)
:   video(input_path),
    video_info(video.get_video_info()),
    main_hbox(Gtk::Orientation::HORIZONTAL),
    label_vbox(Gtk::Orientation::VERTICAL)
{
    set_margin(5);

    // "Rukojeť" naznačující přesouvání
    drag_handle_icon.set_from_icon_name("list-drag-handle-symbolic");
    drag_handle_icon.set_margin_start(5);
    drag_handle_icon.set_margin_end(5);
    drag_handle_icon.add_css_class("dim-label");
    drag_handle_icon.set_pixel_size(16);

    // Obrázek videa
    video_thumbnail.set_margin(10);
    video_thumbnail.set_cursor(Gdk::Cursor::create("grab"));

    string icon_name = fs::path(input_path).stem().generic_string() + ".jpg";
    fs::path thumbnail_path("/tmp/vidcom/thumnail/thumb_" + icon_name);

    if (!filesystem::exists(thumbnail_path.parent_path()))
    {
        filesystem::create_directories(thumbnail_path.parent_path());
    }

    string thumnail_command = "ffmpeg -y -i '" + input_path + "' -ss 1 -vframes 1 -s 64x64 '" + thumbnail_path.generic_string() + "' > /dev/null 2>&1";
    int thumnail_gen_success = std::system(thumnail_command.c_str());

    if (thumnail_gen_success == 0)
    {
        video_thumbnail.set(thumbnail_path.generic_string());
    }
    else 
    {
        video_thumbnail.set_from_icon_name("video-x-generic-symbolic");
    }

    video_thumbnail.set_pixel_size(64);

    // Textíky - informace o videu
    video_name_text.set_ellipsize(Pango::EllipsizeMode::END);

    update_labels();

    video_name_text.set_halign(Gtk::Align::START);
    resolution_text.set_halign(Gtk::Align::START);
    duration_text.set_halign(Gtk::Align::START);
    framerate_text.set_halign(Gtk::Align::START);
    mode_text.set_halign(Gtk::Align::START);
    mode_text.set_ellipsize(Pango::EllipsizeMode::MIDDLE);

    resolution_text.add_css_class("caption");
    duration_text.add_css_class("caption");
    framerate_text.add_css_class("caption");
    mode_text.add_css_class("caption");
    mode_text.add_css_class("accent");

    label_vbox.append(video_name_text);
    label_vbox.append(resolution_text);
    label_vbox.append(duration_text);
    label_vbox.append(framerate_text);
    label_vbox.append(mode_text);

    // Tlačítko smazat
    remove_element_button.set_icon_name("user-trash-symbolic");
    remove_element_button.get_style_context() -> add_class("destructive-action");
    remove_element_button.set_hexpand();
    remove_element_button.set_halign(Gtk::Align::END);
    remove_element_button.add_css_class("flat");
    remove_element_button.signal_clicked().connect(sigc::mem_fun(*this, &VideoElement::on_remove_clicked));

    // Všechno to přidat do finální položky
    main_hbox.append(drag_handle_icon);
    main_hbox.append(video_thumbnail);
    main_hbox.append(label_vbox);
    main_hbox.append(remove_element_button);
    main_hbox.set_margin(10);

    // Změna pořadí prvků ve frontě
    // chycení
    drag_source = Gtk::DragSource::create();
    drag_source -> set_actions(Gdk::DragAction::MOVE);
    drag_source -> signal_prepare().connect(sigc::mem_fun(*this, &VideoElement::on_drag_prepare), false);
    drag_source -> signal_drag_begin().connect(sigc::mem_fun(*this, &VideoElement::on_drag_begin), false);

    // puštění
    drop_target = Gtk::DropTarget::create(G_TYPE_POINTER, Gdk::DragAction::MOVE);
    drop_target -> signal_drop().connect(sigc::mem_fun(*this, &VideoElement::on_drop), false);

    set_child(main_hbox);
    add_controller(drag_source);
    add_controller(drop_target);

    add_css_class("card");
}

VideoElement::~VideoElement()
{}

Glib::RefPtr<Gdk::ContentProvider> VideoElement::on_drag_prepare(double, double)
{
    // Vytvořím hodnotu, inicializuju jí, nahraju do ní ten prvek a vrátím to

    auto value = Glib::Value<gpointer>();
    value.init(G_TYPE_POINTER);
    value.set(static_cast<gpointer>(this));

    return Gdk::ContentProvider::create(value);
}

void VideoElement::on_drag_begin(const Glib::RefPtr<Gdk::Drag>& drag)
{
    // Zpětná vazba při přetahování

    auto paintable = Gtk::WidgetPaintable::create(*Glib::wrap(GTK_WIDGET(gobj())));
    drag -> set_hotspot(0, 0);
    drag_source -> set_icon(paintable, 0, 0);
}

bool VideoElement::on_drop(const Glib::ValueBase& value, double, double)
{
    // Pokud to nemá tu hodnotu, skončíme hned
    if (!G_VALUE_HOLDS(value.gobj(), G_TYPE_POINTER))
    {
        return false;
    }

    // Získat ten prvek
    auto element = static_cast<VideoElement *>(g_value_get_pointer(value.gobj()));

    // Nemůžu to přetáhnout do sebe ani do neexistujícího prvku
    if (!element || element == this)
    {
        return false;
    }

    // Získat cílový řádek a zdrojový řádek (zase to přetypování z Widget na ListBoxRow)
    auto target_row = dynamic_cast<Gtk::ListBoxRow *>(get_parent());
    auto source_row = dynamic_cast<Gtk::ListBoxRow *>(element -> get_parent());

    if (!target_row || !source_row)
    {
        return false;
    }

    // Získá tu frontu
    auto listbox = dynamic_cast<Gtk::ListBox *>(target_row -> get_parent());
    
    if (!listbox)
    {
        return false;
    }

    // Pozice prvků
    int target_index = target_row -> get_index();
    int source_index = source_row -> get_index();
    
    if (target_index == source_index)
    {
        return false;
    }

    // Odebrání původního řádku a vložení na cílový
    // g_object_ref zachovává referenci, takže to RefPtr automaticky nesmaže (taková záloha, ale memory-safe)

    g_object_ref(source_row -> gobj());
    listbox -> remove(*source_row);
    listbox -> insert(*source_row, target_index);
    g_object_unref(source_row -> gobj()); // Odebrat referenci, jinak by tam přebývala a nastal by únik paměti

    listbox -> select_row(*source_row);

    return true;
}

void VideoElement::update_labels()
{
    video_name_text.set_markup("<b>" + video_info.path.filename().generic_string() + "</b>");

    if (video.get_downscale_factor() == 1.0 || !video.is_compress_enabled())
    {
        resolution_text.remove_css_class("accent");
        resolution_text.set_text(to_string(video_info.resolution.width) + "x" + to_string(video_info.resolution.height));
    }
    else if (video.is_compress_enabled())
    {
        resolution_text.add_css_class("accent");
        resolution_text.set_text(to_string(video_info.resolution.width) + "x" + to_string(video_info.resolution.height) + " -> " + to_string(int(video_info.resolution.width / video.get_downscale_factor())) + "x" + to_string(int(video_info.resolution.height / video.get_downscale_factor())));
    }

    if (video.get_output_framerate() == int(video_info.framerate) || !video.is_compress_enabled())
    {
        framerate_text.remove_css_class("accent");
        framerate_text.set_text(to_string(video_info.framerate) + " FPS");
    }
    else if (video.is_compress_enabled())
    {
        framerate_text.add_css_class("accent");
        framerate_text.set_text(to_string(video_info.framerate) + " FPS -> " + to_string(video.get_output_framerate()) + " FPS");
    }

    if (video.is_cutting_enabled())
    {
        duration_text.add_css_class("accent");
        duration_text.set_text(format_time(video.get_cut_info().startTime) + " -> " + format_time(video.get_cut_info().endTime));
    }
    else
    {
        duration_text.remove_css_class("accent");
        duration_text.set_text(format_time(video_info.duration));
    }
    
    string mode_text_string;

    if (video.is_compress_enabled())
    {
        string target_size = to_string(video.get_target_size());

        mode_text_string += "COMPRESS to size "
                         + target_size.erase(target_size.find(",") + 2, 20)
                         + " MB with ";
    } 
    else
    {
        mode_text_string += "ARCHIVE with ";
    }

    switch (video.get_codec())
    {
        case AV1:
            mode_text_string += "AV1";
            break;

        case HEVC:
            mode_text_string += "HEVC";
            break;

        case VP9:
            mode_text_string += "VP9";
            break;
    }
    mode_text_string += "";

    mode_text.set_markup(mode_text_string);
}

void VideoElement::on_remove_clicked()
{
    // Pošle sebe signálem do fronty ke smazání
    signal_remove.emit(this);
}

