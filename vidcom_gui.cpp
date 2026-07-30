#include "adwaita.h"
#include "giomm/settings.h"
#include "glibmm/refptr.h"
#include "gtkmm/application.h"
#include "src/cli/cli.h"
#include "src/gui/headers/gui.h"

#define PROJECT_NAME "vidcom-gui"
Glib::RefPtr<Gio::Settings> SETTINGS;

int main(int argc, char **argv) 
{
    Gio::init();
    SETTINGS = Gio::Settings::create(
        "io.github.seja_arctic_fox.vidcom"
    );
    
    if (argc == 1)
    {
        adw_init();
        auto app = Gtk::Application::create("io.github.seja_arctic_fox.vidcom");
        return app -> make_window_and_run<MainWindow>(argc, argv);
    }
    
    return CLI::parse_arguments(argc, argv);
}
