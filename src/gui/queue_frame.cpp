#include "gio/gio.h"
#include "giomm/liststore.h"
#include "glib-object.h"
#include "glibmm/error.h"
#include "glibmm/main.h"
#include "glibmm/refptr.h"
#include "glibmm/ustring.h"
#include "glibmm/value.h"
#include "gtkmm/alertdialog.h"
#include "gtkmm/droptarget.h"
#include "gtkmm/enums.h"
#include "gtkmm/error.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/filefilter.h"
#include "gtkmm/object.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include "gui.h"
#include "sigc++/functors/mem_fun.h"
#include <iostream>
#include <string>
#include <gdk/gdk.h>
#include <vector>
#include "../cli/cli.h"

QueueFrame::QueueFrame()
:   scrolled_window(),
    video_listbox(),
    empty_queue_label(),
    import_video_button("Add video(s)"),
    header_box(),
    clear_queue_box(Gtk::Orientation::HORIZONTAL),
    select_all_box(Gtk::Orientation::HORIZONTAL),
    clear_queue_text("Clear queue"),
    select_all_text("Select all")
{
    set_size_request(320, -1);
    set_orientation(Gtk::Orientation::VERTICAL);

    // Horní lišta
    select_all_icon.set_from_icon_name("edit-select-all-symbolic");
    select_all_icon.set_margin(5);
    select_all_box.append(select_all_icon);
    select_all_box.append(select_all_text);
    select_all_button.set_child(select_all_box);
    select_all_button.set_can_target(false);

    clear_queue_icon.set_from_icon_name("edit-clear-all-symbolic");
    clear_queue_icon.set_margin(5);
    clear_queue_box.append(clear_queue_icon);
    clear_queue_box.append(clear_queue_text);
    clear_queue_button.set_child(clear_queue_box);
    clear_queue_button.set_can_target(false);

    clear_queue_button.signal_clicked().connect(sigc::mem_fun(*this, &QueueFrame::on_clear_clicked));
    select_all_button.signal_toggled().connect(sigc::mem_fun(*this, &QueueFrame::on_select_all_clicked));

    header_box.set_margin(10);
    header_box.set_spacing(5);
    header_box.set_halign(Gtk::Align::CENTER);
    header_box.append(select_all_button);
    header_box.append(clear_queue_button);

    // Prázdná fronta
    empty_queue_label.set_markup("<span size='large'><b>Drag your videos here</b></span>");
    empty_queue_box.set_valign(Gtk::Align::CENTER);
    empty_queue_box.set_halign(Gtk::Align::CENTER);
    empty_queue_box.set_orientation(Gtk::Orientation::VERTICAL);
    empty_queue_icon.set_from_icon_name("camera-video-symbolic");
    empty_queue_icon.set_pixel_size(128);
    empty_queue_icon.add_css_class("dimmed");
    empty_queue_caption.set_text("...or click the button below to import them. ");
    empty_queue_caption.add_css_class("caption");
    empty_queue_icon.set_margin(20);
    empty_queue_label.set_margin(10);

    empty_queue_box.append(empty_queue_icon);
    empty_queue_box.append(empty_queue_label);
    empty_queue_box.append(empty_queue_caption);

    // Fronta videí
    video_listbox.set_selection_mode(Gtk::SelectionMode::SINGLE);
    video_listbox.add_css_class("boxed-list");
    video_listbox.set_margin(10);
    video_listbox.set_placeholder(empty_queue_box);
    video_listbox.signal_row_selected().connect(sigc::mem_fun(*this, &QueueFrame::on_row_selected));

    // Přidat do okna s posuvníkem
    scrolled_window.set_child(video_listbox);
    scrolled_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scrolled_window.set_expand();

    // Spodní lišta
    import_video_button.set_margin(10);
    import_video_button.add_css_class("suggested-action");
    import_video_button.signal_clicked().connect(sigc::mem_fun(*this, &QueueFrame::on_import_video_clicked));
    footer_box.set_halign(Gtk::Align::CENTER);
    footer_box.append(import_video_button);

    // Drag and drop
    drag_and_drop_target = Gtk::DropTarget::create(gdk_file_list_get_type(), Gdk::DragAction::COPY);
    drag_and_drop_target -> signal_drop().connect(sigc::mem_fun(*this, &QueueFrame::on_drop), false);

    // Přidat věci do boxu fronty
    append(header_box);
    append(scrolled_window);
    append(footer_box);
    add_controller(drag_and_drop_target);
}

QueueFrame::~QueueFrame()
{}

std::vector<Video *> QueueFrame::get_all_videos()
{
    std::vector<Video *> all_elements;

    // Projít všechny řádky v ListBoxu
    // U get_children to bohužel nefunguje, dříve nebo později to vyvolá SIGSEGV

    int index = 0;
    while (auto row = video_listbox.get_row_at_index(index))
    {
        Video * element = &(dynamic_cast<VideoElement*>(row -> get_child()) -> video);
        if (element)
        {
            all_elements.push_back(element);
        }
        index++;
    }
    
    return all_elements;
}

void QueueFrame::file_picker_add_videos(const Glib::RefPtr<Gio::AsyncResult>& result, Glib::RefPtr<Gtk::FileDialog> file_picker)
{
    try
    {
        auto files = file_picker -> open_multiple_finish(result);
        
        // Shared pointer pro kontrolu stavu ve voláních
        // Stavová proměnná musí přežít několik pozdějích volání toho idle handleru
        // po skončení této metody
        auto state = std::make_shared<std::pair<std::vector<std::string>, size_t>>();
        state -> second = 0;

        for (guint i = 0; i < files.size(); i++)
        {
            auto file = files.at(i);

            if (file)
            {
                auto path = file -> get_path();

                if (!path.empty())
                {
                    state -> first.push_back(path);
                }
            }
        }
        
        if (state -> first.empty())
        {
            signal_loading_videos.emit(false);
            return;
        }
        
        signal_loading_videos_count.emit(0, (int) state -> first.size());
        
        Glib::signal_idle().connect([this, state]() -> bool
        {
            size_t& i = state -> second;
            auto& paths = state -> first;
            
            if (i < paths.size())
            {
                signal_loading_videos_count.emit((int) i + 1, (int) paths.size());
                add_video(paths[i]);
                i++;
                return true;
            }
            
            signal_loading_videos.emit(false);
            return false;
        });
    }
    catch (const Gtk::DialogError& error)
    {
        if (error.code() != Gtk::DialogError::DISMISSED)
        {
            cerr << YELLOW << "File picker cancelled by user. " << RESET << endl;
        }
        
        signal_loading_videos.emit(false);
    }
    catch (const Glib::Error& error)
    {
        cerr << RED << "Error opening files with file picker! " << error.what() << RESET << endl;

        auto error_dialog = Gtk::AlertDialog::create();
        error_dialog -> set_message("Error opening files with file picker! ");
        error_dialog -> set_detail("There was a problem with opening files: \n\n");
        error_dialog -> set_buttons({"Okay"});
        error_dialog -> set_cancel_button(0);

        error_dialog -> show(* dynamic_cast<Gtk::Window *>(get_root()));
        signal_loading_videos.emit(false);
    }
}


void QueueFrame::on_import_video_clicked()
{
    signal_loading_videos.emit(true);
    auto file_picker = Gtk::FileDialog::create();
    file_picker -> set_title("Select video(s) to import");
    file_picker -> set_modal();

    // Filtry videí
    auto video_filter = Gtk::FileFilter::create();
    video_filter -> set_name("Video files");
    video_filter -> add_mime_type("video/*");

    auto all_files_filter = Gtk::FileFilter::create();
    all_files_filter -> set_name("All files");
    all_files_filter -> add_pattern("*");

    auto filter_list = Gio::ListStore<Gtk::FileFilter>::create();
    filter_list -> append(video_filter);
    filter_list -> append(all_files_filter);
    file_picker -> set_filters(filter_list);
    file_picker -> set_default_filter(video_filter);

    // Otevření file pickeru
    file_picker -> open_multiple(* dynamic_cast<Gtk::Window *>(get_root()), sigc::bind(sigc::mem_fun(* this, &QueueFrame::file_picker_add_videos), file_picker));
}

void QueueFrame::on_row_selected(Gtk::ListBoxRow * row)
{
    if (video_listbox.get_selection_mode() == Gtk::SelectionMode::SINGLE && select_all_button.get_active())
    {
        select_all_button.set_active(false);
    }

    if (row)
    {
        VideoElement * element = dynamic_cast<VideoElement *>(row -> get_child());

        if (element)
        {
            signal_video_selected.emit(element);
        }
    }

}

void QueueFrame::error_dialog_not_a_video()
{
    auto dialog = Gtk::AlertDialog::create();
    dialog -> set_message("Imported file is not a video!");
    dialog -> set_detail("Input file is not a video file or no video streams were found. ");
    dialog -> set_buttons({"Got it"});
    dialog -> set_cancel_button(0);

    dialog -> show(* dynamic_cast<Gtk::Window *>(get_root()));
}

void QueueFrame::add_video(const std::string& input_path)
{
    select_all_button.set_can_target();
    select_all_button.set_active(false);
    clear_queue_button.set_can_target();

    VideoElement * new_video = Gtk::make_managed<VideoElement>(input_path);
    float duration = new_video->video.get_video_info().duration;

    if (duration == -1)
    {
        error_dialog_not_a_video();
        return;
    }

    // Signál odstranění odstraní video ze seznamu
    new_video -> signal_remove.connect([this, new_video](VideoElement *)
        {
            // Nemažu přímo ten prvek, ale objekt řádku, ve kterém je uložen prvek
            auto row = new_video -> get_parent();
            if (row)
            {
                video_listbox.remove(* row);

                if (!video_listbox.get_row_at_index(0))
                {
                    signal_nothing_selected.emit();
                    select_all_button.set_active(false);
                    select_all_button.set_can_target(false);
                    clear_queue_button.set_can_target(false);
                }
            }
        }
    );

    video_listbox.append(* new_video);
    video_listbox.select_row(*video_listbox.get_row_at_index(0));
}

void QueueFrame::on_clear_clicked()
{
    video_listbox.remove_all();
    select_all_button.set_active(false);
    select_all_button.set_can_target(false);
    clear_queue_button.set_can_target(false);
    video_listbox.set_selection_mode(Gtk::SelectionMode::SINGLE);

    signal_nothing_selected.emit();

    // Nastavit zpět placeholder
    video_listbox.set_placeholder(empty_queue_box);
}

void QueueFrame::on_select_all_clicked()
{
    if (select_all_button.get_active())
    {
        video_listbox.set_selection_mode(Gtk::SelectionMode::MULTIPLE);
        video_listbox.select_all();
        vector<VideoElement *> all_video_elements;
        auto all_rows = video_listbox.get_selected_rows();

        for (auto row : all_rows)
        {
            VideoElement * element = dynamic_cast<VideoElement *>(row -> get_child());
            if (element)
            {
                all_video_elements.push_back(element);
            }
        }

        signal_all_videos_selected.emit(all_video_elements);
    }
    else
    {
        if (video_listbox.get_row_at_index(0))
        {
            video_listbox.set_selection_mode(Gtk::SelectionMode::SINGLE);
            video_listbox.select_row(*video_listbox.get_row_at_index(0));
        }
    }
}

bool QueueFrame::on_drop(const Glib::ValueBase& value, double, double)
{
    // Původní implementaci jsem měl ve vnořených podmínkách
    // To znemožňovalo nahlašování stavu, protože GTK blokovalo překreslování, 
    // dokud se drop akce nedokončila. 
    // Zde je to lépe rozdělené + na konec je idle handler, který to překreslí v mezičasech
    
    // Seznam souborů
    if (!G_VALUE_HOLDS(value.gobj(), gdk_file_list_get_type())) return false;
    
    // Získat ukazatel na seznam souborů
    GdkFileList* file_list = (GdkFileList*)g_value_get_boxed(value.gobj());
    if (!file_list) return false;

    // Sestavit seznam cest napřed
    std::vector<std::string> dropped_videos;
    GSList* list = gdk_file_list_get_files(file_list);
    for (GSList* l = list; l != nullptr; l = l->next)
    {
        char* path_c = g_file_get_path(G_FILE(l->data));
        if (path_c)
        {
            dropped_videos.push_back(std::string(path_c));
            g_free(path_c);
        }
    }
    if (dropped_videos.empty()) return false;
    
    // Nahlášení stavu do runneru
    signal_loading_videos.emit(true);

    // Sdílený stav pro idle handler
    // Je třeba, protože musí přežít několik pozdějších volání idle handleru
    // po skončení této metody
    auto state = std::make_shared<std::pair<std::vector<std::string>, size_t>>
    (
        std::move(dropped_videos), 0
    );

    // Musím použít idle handler, protože jinak mi to nešlo
    // Akce puštění souboru totiž blokovala všechny signály, dokud se nedokončila
    // Videa se zpracovávají po jednom. Když se vrátí true, idle handler se vykoná znovu
    Glib::signal_idle().connect([this, state]() -> bool
    {
        size_t& i = state->second;
        auto& paths = state->first;

        if (i < paths.size())
        {
            signal_loading_videos_count.emit((int)i + 1, (int)paths.size());
            add_video(paths[i]);
            i++;
            return true;
        }

        // Hotovo, už mě nevolej
        signal_loading_videos.emit(false);
        return false;
    });

    return true;
}