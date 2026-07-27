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
        auto app = Gtk::Application::create("io.github.seja_arctic_fox.vidcom");
        return app -> make_window_and_run<MainWindow>(argc, argv);
    }
}
