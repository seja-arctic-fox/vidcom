#include "adwaita.h"
#include "giomm/settings.h"
#include "glibmm/refptr.h"
#include "giomm/application.h"
#include "gdk/gdk.h"
#include "gtk/gtk.h"
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
    
        GtkIconTheme * theme = gtk_icon_theme_get_for_display(
            gdk_display_get_default()
        );
        gtk_icon_theme_add_search_path(theme, "./data/icons");
        
        int result = app -> run(argc, argv);
        delete window;
        return result;
    }
}
