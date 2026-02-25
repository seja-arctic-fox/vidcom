#include "giomm/appinfo.h"
#include "giomm/file.h"
#include "glibmm/error.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/enums.h"
#include "gtkmm/image.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "gtkmm/scrolledwindow.h"
#include "gui.h"
#include "pangomm/layout.h"
#include <filesystem>
#include <iostream>
#include "../cli/cli.h"

ResultsDialog::ResultsDialog(Gtk::Window& parent, const std::vector<EncodingResult>& encoding_results)
:   encoding_results(encoding_results),
    scrolled_window(),
    results_listbox(),
    window_content(Gtk::Orientation::VERTICAL),
    ok_button("Okay")
{
    set_transient_for(parent);
    set_modal();
    set_title("Encoding Results");
    set_child(scrolled_window);
    set_size_request(960, 540);

    results_listbox.set_expand();
    results_listbox.add_css_class("card");
    results_listbox.add_css_class("flat");
    load_results();
    results_listbox.set_selection_mode(Gtk::SelectionMode::NONE);

    ok_button.set_margin(20);

    window_content.set_margin(20);
    window_content.append(results_listbox);
    window_content.append(ok_button);

    scrolled_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scrolled_window.set_expand();
    scrolled_window.set_child(window_content);

    ok_button.signal_clicked().connect([this]()
        {
            destroy();
        }
    );
}

ResultsDialog::~ResultsDialog()
{}

void ResultsDialog::load_results()
{
    for (auto result : encoding_results)
    {
        auto row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        row -> set_margin(5);

        auto video_name = Gtk::make_managed<Gtk::Label>(result.video_name);
        video_name -> add_css_class("heading");
        video_name -> set_ellipsize(Pango::EllipsizeMode::MIDDLE);
        video_name -> set_margin_start(10);
        video_name -> set_margin_end(10);

        auto status_icon = Gtk::make_managed<Gtk::Image>();
        auto status_text = Gtk::make_managed<Gtk::Label>();
        status_icon -> set_margin_start(10);
        status_icon -> set_margin_end(10);
        status_text -> set_margin_start(10);
        status_text -> set_margin_end(10);

        if (result.exit_status == 0)
        {
            status_icon -> add_css_class("success");
            status_text -> add_css_class("success");

            status_icon -> set_from_icon_name("object-select-symbolic");
            status_text -> set_markup("<b>Succesfull</b>");
        }
        else if (result.exit_status == -2 || result.exit_status == -1)
        {
            status_icon -> add_css_class("warning");
            status_text -> add_css_class("warning");

            status_icon -> set_from_icon_name("dialog-warning-symbolic");
            status_text -> set_markup("<b>Cancelled</b>");
        }
        else
        {
            status_icon -> add_css_class("error");
            status_text -> add_css_class("error");

            status_icon -> set_from_icon_name("process-stop-symbolic");
            status_text -> set_markup("<b>Error: Code " + to_string(result.exit_status) + "</b>");
        }

        auto output_button = Gtk::make_managed<Gtk::Button>("Show video");
        output_button -> add_css_class("suggested-action");
        output_button -> add_css_class("pill");
        output_button -> set_margin_start(10);
        output_button -> set_margin_end(10);

        if (result.was_cancelled || result.exit_status != 0)
        {
            output_button -> set_sensitive(false);
        }

        output_button -> signal_clicked().connect(([result]() 
            {
                try
                {
                    auto file = Gio::File::create_for_path(result.video_path);
                    Gio::AppInfo::launch_default_for_uri(file -> get_uri());
                }
                catch (const Glib::Error& ex)
                {
                    cerr << RED << "Error opening file (" << ex.what() << ")" << RESET << endl;
                }
            }
        ));

        auto output_folder_button = Gtk::make_managed<Gtk::Button>("Show output folder");
        output_folder_button -> add_css_class("suggested-action");
        output_folder_button -> add_css_class("pill");
        output_folder_button -> set_margin_start(10);
        output_folder_button -> set_margin_end(10);

        output_folder_button -> signal_clicked().connect(([result]() 
            {
                try
                {
                    auto file = Gio::File::create_for_path(filesystem::path(result.video_path).parent_path().generic_string());
                    Gio::AppInfo::launch_default_for_uri(file -> get_uri());
                }
                catch (const Glib::Error& ex)
                {
                    cerr << RED << "Error opening folder (" << ex.what() << ")" << RESET << endl;
                }
            }
        ));

        auto box_rest = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        box_rest -> set_margin(5);
        box_rest -> set_expand();
        box_rest -> set_halign(Gtk::Align::END);
        box_rest -> append(*status_icon);
        box_rest -> append(*status_text);
        box_rest -> append(*output_button);
        box_rest -> append(*output_folder_button);

        row -> append(*video_name);
        row -> append(*box_rest);

        results_listbox.append(*row);
    }
}