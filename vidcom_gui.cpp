/*
    VidCom, an utility for video archival and compression
    Copyright (C) 2026  seja-arctic-fox <seja.czstudio@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "src/gui/headers/hide_console.h"
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
    #ifdef _WIN32
    hide_console_for_children();
    #endif
    
    Gio::init();
    SETTINGS = Gio::Settings::create(
        "io.github.seja_arctic_fox.vidcom"
    );
    
    #ifdef _WIN32
    
    if (argc > 1)
        return CLI::parse_arguments(argc, argv);
    
    #else
    
    if (argc > 1 && (string(argv[1]) == "-cli" || string(argv[1]) == "-h"))
        return CLI::parse_arguments(argc, argv);
    
    #endif
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
