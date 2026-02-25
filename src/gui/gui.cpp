#include "gui.h"
#include "gtkmm/enums.h"
#include "gtkmm/headerbar.h"
#include "gtkmm/object.h"
#include "sigc++/functors/mem_fun.h"
#include <mutex>
#include <thread>
#include <vector>

MainWindow::MainWindow()
:   header_bar(),
    runner_panel(),
    options_page(),
    is_encoding(false)
{
    set_title("VidCom 0.8 GUI");
    set_default_size(960, 540);

    // Horní lišta
    header_bar.set_show_title_buttons();
    header_bar.set_title_widget(runner_panel);
    set_titlebar(header_bar);

    // Rozdělené okno na dvě části
    paned.set_shrink_start_child(false);
    paned.set_orientation(Gtk::Orientation::HORIZONTAL);
    paned.set_start_child(video_queue);
    paned.set_end_child(options_page);
    paned.set_resize_end_child();
    paned.set_shrink_end_child(false);
    paned.set_position(320);
    set_child(paned);

    // Propojení signálů pro aktualizaci nastavení videa
    video_queue.signal_video_selected.connect(sigc::mem_fun(options_page, &VideoSettings_VBox::read_video_options));
    video_queue.signal_all_videos_selected.connect(sigc::mem_fun(options_page, &VideoSettings_VBox::read_video_vector_options));
    video_queue.signal_nothing_selected.connect(sigc::mem_fun(options_page, &VideoSettings_VBox::no_video_selected));

    // Signály pro začátek a zastavení kódování
    runner_panel.signal_start_encoding.connect(sigc::mem_fun(*this, &MainWindow::start_encoding));
    runner_panel.signal_stop_encoding.connect(sigc::mem_fun(*this, &MainWindow::stop_encoding));

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

void MainWindow::start_encoding()
{
    if (is_encoding.load())
    {
        return;
    }

    // Když je prázdná fronta, ukázat hlášení a konec
    if (video_queue.get_all_videos().empty())
    {
        auto dialog = Gtk::AlertDialog::create();
        dialog -> set_message("No videos in queue");
        dialog -> set_detail("Video queue is empty. Start with adding some videos into the queue. ");
        dialog -> set_buttons({"Okay"});
        dialog -> set_modal();
        dialog -> show(*this);
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

        video->test_commands();
        int exit_code = video -> encode("", "", progress_callback);

        EncodingResult result;
        result.video_name = video -> get_video_info().path.filename();
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
}

void MainWindow::on_progress_update()
{
    std::lock_guard<std::mutex> lock(encoding_mutex);
    runner_panel.update_progress(current_progress);
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

    show_results_dialog();
}

void MainWindow::show_results_dialog()
{
    std::lock_guard<std::mutex> lock(encoding_mutex);

    auto result_dialog = Gtk::make_managed<ResultsDialog>(*this, encoding_results);
    result_dialog -> present();
}