#include "gtkmm/button.h"
#include "gtkmm/enums.h"
#include "gtkmm/object.h"
#include "gtkmm/scrolledwindow.h"
#include "gui.h"

ResultsPage::ResultsPage()
:   
    result_label("Your videos have been compressed. Click the items below to see them. "),
    results_listbox(),
    ok_button("Okay")
{
    result_label.set_margin(10);
    result_label.set_halign(Gtk::Align::CENTER);
    result_label.add_css_class("heading");
    result_label.set_ellipsize(Pango::EllipsizeMode::END);
    
    results_listbox.set_expand();
    results_listbox.add_css_class("card");
    results_listbox.add_css_class("flat");
    results_listbox.set_selection_mode(Gtk::SelectionMode::NONE);
    results_listbox.signal_row_activated().connect([](Gtk::ListBoxRow * row) { dynamic_cast<ResultRow *>(row -> get_child()) -> open_video(); });

    ok_button.set_margin(20);
    ok_button.set_hexpand(false);
    ok_button.add_css_class("suggested-action");
    ok_button.add_css_class("pill");
    ok_button.signal_clicked().connect([this]() { signal_close_results.emit(); });

    scrolled_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scrolled_window.set_expand();
    scrolled_window.set_child(results_listbox);
    
    set_margin(20);
    set_orientation(Gtk::Orientation::VERTICAL);
    append(result_label);
    append(scrolled_window);
    append(ok_button);
}

ResultsPage::~ResultsPage()
{}

void ResultsPage::load_results(std::vector<EncodingResult> encoding_results)
{
    results_listbox.remove_all();
    
    for (auto result : encoding_results)
    {
        auto row = Gtk::make_managed<ResultRow>(result.video_path, result.exit_status);
        results_listbox.append(*row);
    }
}
