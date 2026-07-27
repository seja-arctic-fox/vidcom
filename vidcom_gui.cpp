#include "adwaita.h"
#include "giomm/application.h"
#include "gtkmm/application.h"
#include "src/cli/cli.h"
#include "src/gui/headers/gui.h"

#define PROJECT_NAME "vidcom-gui"

int main(int argc, char **argv) 
{
    if (argc > 1 && (string(argv[1]) == "-cli" || string(argv[1]) == "-h"))
        return CLI::parse_arguments(argc, argv);
    else
    {
        adw_init();
        auto app = Gtk::Application::create(
            "io.github.seja_arctic_fox.vidcom", 
            Gio::Application::Flags::HANDLES_OPEN
        );
        
        MainWindow * window = nullptr;
        
        app -> signal_activate().connect([&app, &window]() {
            if (!window) 
            {
                window = new MainWindow(); app -> add_window(*window);
            }
            window -> present();
        });
    
        app -> signal_open().connect([&app, &window]
            (const Gio::Application::type_vec_files& files, const Glib::ustring&) 
        {
                if (!window) 
                {
                    window = new MainWindow(); app -> add_window(*window); 
                }
            window -> on_open_videos(files, "");
            window -> present();
        });
    
        int result = app -> run(argc, argv);
        delete window;
        return result;
    }
}
