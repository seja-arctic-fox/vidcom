#include "adwaita.h"
#include "gtkmm/application.h"
#include "src/cli/cli.h"
#include "src/gui/gui.h"

#define PROJECT_NAME "vidcom-gui"

int main(int argc, char **argv) 
{

    if (argc == 1)
    {
        adw_init();
        auto app = Gtk::Application::create("cz.seja.vidcom");
        return app -> make_window_and_run<MainWindow>(argc, argv);
    }
    
    return CLI::parse_arguments(argc, argv);
}
