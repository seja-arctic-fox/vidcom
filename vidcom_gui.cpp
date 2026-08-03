#include "adwaita.h"
#include "gdk/gdk.h"
#include "gtk/gtk.h"
#include "gtkmm/application.h"
#include "src/cli/cli.h"
#include "src/gui/headers/gui.h"

#define PROJECT_NAME "vidcom-gui"

int main(int argc, char **argv) 
{

    if (argc == 1)
    {
        adw_init();
        GtkIconTheme * theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
        gtk_icon_theme_add_search_path(theme, "./data/icons");
        auto app = Gtk::Application::create("io.github.seja_arctic_fox.vidcom");
        return app -> make_window_and_run<MainWindow>(argc, argv);
    }
    
    return CLI::parse_arguments(argc, argv);
}
