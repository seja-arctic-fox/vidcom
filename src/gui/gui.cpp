#include "gui.h"
#include "adwaita.h"
#include "gtkmm/headerbar.h"
#include "gtkmm/object.h"
#include "sigc++/functors/mem_fun.h"
#include "src/cli/cli.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

MainWindow::MainWindow()
:
    header_bar(),
    runner_panel(),
    options_page(),
    is_encoding(false)
{
    set_title("VidCom");
    set_default_size(960, 540);
    gtk_window_set_titlebar(GTK_WINDOW(gobj()), gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    
    // Hlavní nabídka
    main_menu = Gio::Menu::create();
    main_menu -> append("Settings", "app.preferences");
    main_menu -> append("Keyboard Shortcuts", "app.shortcuts");
    main_menu -> append_section({}, []
        {
            auto s = Gio::Menu::create();
            s -> append("About VidCom");
            return s;
        }());
    
    menu_button.set_icon_name("open-menu-symbolic");;
    menu_button.set_menu_model(main_menu);
    
    // Přidat video
    add_videos_button.set_icon_name("tab-new-symbolic");
    add_videos_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_import_video_clicked));
    
    // Postranní panel
    sidebar_header = ADW_HEADER_BAR(adw_header_bar_new());
    adw_header_bar_pack_start(sidebar_header, GTK_WIDGET(add_videos_button.gobj()));
    adw_header_bar_pack_end(sidebar_header, GTK_WIDGET(menu_button.gobj()));
    
    sidebar_view = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    adw_toolbar_view_add_top_bar(sidebar_view, GTK_WIDGET(sidebar_header));
    adw_toolbar_view_set_content(sidebar_view, GTK_WIDGET(video_queue.gobj()));
    
    // Hlavní část okna
    content_view = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    adw_toolbar_view_add_top_bar(content_view, GTK_WIDGET(runner_panel.gobj()));
    
    toast_overlay = ADW_TOAST_OVERLAY(adw_toast_overlay_new());
    adw_toast_overlay_set_child(toast_overlay, GTK_WIDGET(options_page.gobj()));
    adw_toolbar_view_set_content(content_view, GTK_WIDGET(toast_overlay));
    
    // Složení hlavní části a postranního panelu
    split_view = ADW_OVERLAY_SPLIT_VIEW(adw_overlay_split_view_new());
    adw_overlay_split_view_set_sidebar(split_view, GTK_WIDGET(sidebar_view));
    adw_overlay_split_view_set_content(split_view, GTK_WIDGET(content_view));
    adw_overlay_split_view_set_sidebar_position(split_view, GTK_PACK_START);
    adw_overlay_split_view_set_min_sidebar_width(split_view, 250.0);
    
    set_child(*Glib::wrap(GTK_WIDGET(split_view)));

    signal_realize().connect([this]()
        {
            get_surface() -> signal_layout().connect(
                sigc::mem_fun(*this, &MainWindow::on_window_resize)
            );
        }
    );

    // Signál pro zobrazení/skrývání fronty
    runner_panel.signal_toggle_queue.connect([this]()
        { adw_overlay_split_view_set_show_sidebar(split_view, true); });
    
    // Signály pro změny názvu videa v runneru
    video_queue.signal_video_selected.connect(sigc::mem_fun(runner_panel, &RunnerPanel::set_title));
    video_queue.signal_all_videos_selected.connect(sigc::mem_fun(runner_panel, &RunnerPanel::set_title_multiple));
    video_queue.signal_nothing_selected.connect(sigc::mem_fun(runner_panel, &RunnerPanel::clear_title));
    
    // Propojení signálů pro aktualizaci nastavení videa
    video_queue.signal_video_selected.connect(sigc::mem_fun(options_page, &VideoSettings_VBox::read_video_options));
    video_queue.signal_all_videos_selected.connect(sigc::mem_fun(options_page, &VideoSettings_VBox::read_video_vector_options));
    video_queue.signal_nothing_selected.connect(sigc::mem_fun(options_page, &VideoSettings_VBox::no_video_selected));

    // Signály pro začátek a zastavení kódování, načítání videí do fronty
    runner_panel.signal_start_encoding.connect(sigc::mem_fun(*this, &MainWindow::start_encoding));
    runner_panel.signal_stop_encoding.connect(sigc::mem_fun(*this, &MainWindow::stop_encoding));
    video_queue.signal_loading_videos.connect(sigc::mem_fun(runner_panel, &RunnerPanel::set_loading_state));
    video_queue.signal_loading_videos_count.connect(sigc::mem_fun(runner_panel, &RunnerPanel::update_loading_progress));
    signal_loading_videos.connect(sigc::mem_fun(runner_panel, &RunnerPanel::set_loading_state));
    signal_loading_videos_count.connect(sigc::mem_fun(runner_panel, &RunnerPanel::update_loading_progress));

    // Komunikace mezi vlákny
    progress_dispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_progress_update));
    completion_dispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_encoding_complete));
}

MainWindow::~MainWindow()
{
    if (encoding_thread.joinable())
    {
        is_encoding.store(false);
        encoding_thread.join();
    }
}

void MainWindow::on_window_resize(int width, int)
{
    if (width < 860 && !adw_overlay_split_view_get_collapsed(split_view))
    {
        adw_overlay_split_view_set_collapsed(split_view, true);
        adw_overlay_split_view_set_enable_hide_gesture(split_view, true);
        adw_overlay_split_view_set_enable_show_gesture(split_view, true);
        runner_panel.show_queue_button(true);
    }
    else if (width >= 860)
    {
        adw_overlay_split_view_set_collapsed(split_view, false);
        adw_overlay_split_view_set_enable_hide_gesture(split_view, false);
        adw_overlay_split_view_set_enable_show_gesture(split_view, false);
        runner_panel.show_queue_button(false);
    }
}

void MainWindow::show_toast(char const * message)
{
    AdwToast * toast = adw_toast_new(message);
    adw_toast_set_timeout(toast, 5);
    adw_toast_overlay_add_toast(toast_overlay, toast);
}

void MainWindow::start_encoding()
{
    if (is_encoding.load())
    {
        return;
    }
    // Vyčistit předchozí výsledky
    {
        std::lock_guard<std::mutex> lock(encoding_mutex);
        encoding_results.clear();
    }

    // Start kódování
    runner_panel.set_encoding_state(true);
    paned.set_sensitive(false);

    is_encoding.store(true);

    if (encoding_thread.joinable())
    {
        encoding_thread.join();
    }

    encoding_thread = std::thread(&MainWindow::encoding_worker, this);
}

void MainWindow::stop_encoding()
{
    if (!is_encoding.load())
    {
        return;
    }

    // Zastavit kódování
    is_encoding.store(false);
    runner_panel.update_status("Cancelling...");
    runner_panel.block_encoding_button(true);

    // Zrušit kódování pro všechna videa
    std::vector<Video *> all_videos = video_queue.get_all_videos();

    for (Video * video : all_videos)
    {
        video -> cancel_encoding();
    }
}

void MainWindow::encoding_worker()
{
    std::vector<Video *> all_videos = video_queue.get_all_videos();
    int total_video_count = all_videos.size();

    for (int i = 0; i < total_video_count && is_encoding.load(); i++)
    {
        Video * video = all_videos[i];

        // Aktualizace postupu pro nové video
        {
            std::lock_guard<std::mutex> lock(encoding_mutex);
            current_progress.video_name = video -> get_video_info().path.filename();
            current_progress.current_index = i + 1;
            current_progress.total_count = total_video_count;
            current_progress.progress_percent = 0;
            current_progress.current_time = 0.0f;
        }

        progress_dispatcher.emit();
    
        // Callback pro sledování postupu
        auto progress_callback = [this, i, total_video_count](float current_time, int percent)
        {
            std::lock_guard<std::mutex> lock(encoding_mutex);
            current_progress.current_time = current_time;
            current_progress.progress_percent = percent;
            current_progress.current_index = i + 1;
            current_progress.total_count = total_video_count;

            progress_dispatcher.emit();
        };

        video -> test_commands();
        int exit_code = video -> encode("", "", progress_callback);
        
        if (exit_code == -3)
        {
            is_encoding.store(false);
            completion_dispatcher.emit();
            
            EncodingResult result;
            result.video_path = video -> get_output_path();
            result.exit_status = exit_code;
            result.was_cancelled = (exit_code == -2);
            encoding_results.push_back(result);
            
            show_toast("Error while creating output: Insufficient rights");
            show_results_dialog();
            return;
        }

        EncodingResult result;
        result.video_path = video -> get_output_path();
        result.exit_status = exit_code;
        result.was_cancelled = (exit_code == -2);

        {
            std::lock_guard<std::mutex> lock(encoding_mutex);
            encoding_results.push_back(result);
        }

        // Přerušit cyklus, pokud bylo kódování přerušeno
        if (!is_encoding.load())
        {
            break;
        }
    }

    is_encoding.store(false);
    completion_dispatcher.emit();
    show_results_dialog();
}

void MainWindow::on_progress_update()
{
    std::lock_guard<std::mutex> lock(encoding_mutex);
    runner_panel.update_encoding_progress(current_progress);
}

void MainWindow::on_encoding_complete()
{
    runner_panel.set_encoding_state(false);
    runner_panel.block_encoding_button(false);
    paned.set_sensitive();

    if (encoding_thread.joinable())
    {
        encoding_thread.join();
    }
}

void MainWindow::show_results_dialog()
{
    std::lock_guard<std::mutex> lock(encoding_mutex);
    
    // Oznámení
    auto app = Gtk::Application::get_default();
    auto notification_finish = Gio::Notification::create("Encoding finished");
    notification_finish -> set_body("Queued videos have been compressed. Click here to see results");
    app -> send_notification(notification_finish);

    auto result_dialog = Gtk::make_managed<ResultsDialog>(*this, encoding_results);
    result_dialog -> set_visible();
}

void MainWindow::on_import_video_clicked()
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
    file_picker -> open_multiple(* dynamic_cast<Gtk::Window *>(get_root()), sigc::bind(sigc::mem_fun(*this, &MainWindow::file_picker_add_videos), file_picker));
}

void MainWindow::file_picker_add_videos(const Glib::RefPtr<Gio::AsyncResult>& result, Glib::RefPtr<Gtk::FileDialog> file_picker)
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
                video_queue.add_video(paths[i]);
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
        show_toast("Error opening files with file picker!");
        signal_loading_videos.emit(false);
    }
}
