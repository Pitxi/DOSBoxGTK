/**
 * @file
 * MainWindow Class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "mainwindow.h"
#include "config.h"
#include "preferencesdialog.h"
#include "editprofiledialog.h"
#include "resourcemanager.hpp"
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <glibmm/spawn.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/icontheme.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Creates an empty profiles XML file if there is none.
 */
void MainWindow::create_profiles_file()
{
    auto profiles_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_profiles_tv->get_model());

    if (!this->m_profiles_file->query_exists()) {
        profiles_ls->clear();
        this->m_parser.get_document()->create_root_node("profiles");
        this->m_parser.get_document()->write_to_file_formatted(this->m_profiles_file->get_path(), "UTF-8");
    }
}

/**
 * Loads the profiles into the TreeView from the profiles XML file.
 */
void MainWindow::load_profiles()
{
    this->m_parser.parse_file(this->m_profiles_file->get_path());

    auto root_element = this->m_parser.get_document()->get_root_node();
    auto profiles_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_profiles_tv->get_model());

    // Clear the profiles.
    profiles_ls->clear();

    // Load the profiles
    for (xmlpp::Node *child : root_element->get_children("profile")) {
        auto element = static_cast<xmlpp::Element*>(child);
        auto id = element->get_attribute_value("id");
        auto title_element = static_cast<xmlpp::Element*>(*child->get_children("title").begin());
        auto title = title_element->get_child_text()->get_content();

        auto iter = profiles_ls->append();
        iter->set_value(0, title);
        iter->set_value(1, id);
    }
}

/**
 * Gets the IDs of the selected profiles
 * @return std::vector with the requested ids or empty if no profile is
 * selected.
 */
std::vector<Glib::ustring> MainWindow::get_selected_ids() const
{
    auto profiles_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_profiles_tv->get_model());
    auto selected_rows = this->m_profiles_tv->get_selection()->get_selected_rows();
    std::vector<Glib::ustring> ids;

    for (auto row : selected_rows) {
        auto iter = profiles_ls->get_iter(row);
        Glib::ustring id;

        iter->get_value(1, id);
        ids.push_back(id);
    }

    return ids;
}

/**
 * Gets the profile XML element with the given ID.
 * @return Pointer to the requested xmlpp::Element.
 */
xmlpp::Element *MainWindow::get_profile_element(const Glib::ustring &id) const
{
    auto root = this->m_parser.get_document()->get_root_node();
    xmlpp::Element *element = nullptr;

    for (auto child : root->get_children("profile")) {
        auto child_element = static_cast<xmlpp::Element*>(child);

        if (child_element->get_attribute_value("id") == id) {
            element = child_element;
        }
    }

    return element;
}

/**
 * Removes the profile with the given ID and it's associated files.
 * The changes will not take effect until the XML document is saved.
 * @param id Profile ID.
 * @return @c TRUE if the profile gets succesfully removed or @c FALSE
 * otherwise.
 */
bool MainWindow::remove_profile(const Glib::ustring &id)
{
    auto profile = this->get_profile_element(id);

    if (profile != nullptr) {
        auto root = this->m_parser.get_document()->get_root_node();
        auto basedir = this->m_settings->get_string("profiles-path");
        auto config_filename = Glib::build_filename(basedir, Glib::ustring::compose("%1.conf", id)),
             setup_filename  = Glib::build_filename(basedir, Glib::ustring::compose("%1_setup.conf", id));
        auto config_file = Gio::File::create_for_path(config_filename),
             setup_file  = Gio::File::create_for_path(setup_filename);

        if (config_file->query_exists()) {
            config_file->remove();
        }

        if (setup_file->query_exists()) {
            setup_file->remove();
        }

        root->remove_child(profile);
    }

    return profile != nullptr;
}

/**
 * Changes the related controls sensitivity when the TreeView selection changes.
 */
void MainWindow::on_profiles_tv_selection_changed()
{
    auto selected_rows = this->m_profiles_tv->get_selection()->get_selected_rows();
    auto has_setup = false;

    if (selected_rows.size() == 1) {
        auto selected_ids = this->get_selected_ids();
        auto setup_basename = Glib::ustring::compose("%1_setup.conf", selected_ids[0]);
        auto setup_filename = Glib::build_filename(this->m_settings->get_string("profiles-path"), setup_basename);
        auto setup_file = Gio::File::create_for_path(setup_filename);

        has_setup = setup_file->query_exists() && setup_file->query_file_type() == Gio::FILE_TYPE_REGULAR;
    }

    this->m_main_ag->get_action("Edit")->set_sensitive(selected_rows.size() == 1);
    this->m_main_ag->get_action("Run")->set_sensitive(selected_rows.size() == 1);
    this->m_main_ag->get_action("Setup")->set_sensitive(has_setup);
    this->m_main_ag->get_action("Remove")->set_sensitive(selected_rows.size() > 0);
}

/**
 * Execute the the game which profile has been activated with DOSBox.
 * @param path The Gtk::TreePath for the activated row.
 * @param column The Gtk::TreeViewColumn in which the activation occurred.
 */
void MainWindow::on_row_activated(const Gtk::TreePath &path, Gtk::TreeViewColumn *column)
{
    auto run_action = this->m_main_ag->get_action("Run");

    if (run_action->get_sensitive()) {
        run_action->activate();
    }
}

/**
 * Adds a new game profile to the list.
 */
void MainWindow::on_new_activated()
{
    auto resource_path = Glib::ustring::compose("%1gui/editprofiledialog.glade", APP_PATH);
    auto builder = Gtk::Builder::create_from_resource(resource_path);
    EditProfileDialog *dialog = nullptr;

    builder->get_widget_derived("EditProfileDialog", dialog);
    dialog->set_transient_for(*this);

    if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
        dialog->save_profile();
        this->load_profiles();
    }
}

/**
 * Edits the currently selected game profile.
 */
void MainWindow::on_edit_activated()
{
    auto resource_path = Glib::ustring::compose("%1gui/editprofiledialog.glade", APP_PATH);
    auto builder = Gtk::Builder::create_from_resource(resource_path);
    EditProfileDialog *dialog = nullptr;

    builder->get_widget_derived("EditProfileDialog", dialog);
    dialog->set_transient_for(*this);

    dialog->load_profile(this->get_selected_ids()[0]);

    if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
        dialog->save_profile();
        this->load_profiles();
    }
}

/**
 * Remove the selected game profiles.
 */
void MainWindow::on_remove_activated()
{
    auto profiles_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_profiles_tv->get_model());
    auto selected_rows = this->m_profiles_tv->get_selection()->get_selected_rows();

    std::vector<Gtk::TreeIter> iters;

    for (auto row : selected_rows) {
        iters.push_back(profiles_ls->get_iter(row));
    }

    for (auto iter : iters) {
        Glib::ustring id;

        iter->get_value(1, id);

        this->remove_profile(id);
        this->m_parser.get_document()->write_to_file_formatted(this->m_profiles_file->get_path());
        profiles_ls->erase(iter);
    }
}

/**
 * Execute the currently selected game with DOSBox.
 */
void MainWindow::on_run_activated()
{
    Glib::ustring  id               = this->get_selected_ids()[0],
                   basedir          = this->m_settings->get_string("profiles-path"),
                   basename         = Glib::ustring::compose("%1.conf", id),
                   config_file      = Glib::build_filename(basedir, basename),
                   dosbox_exec_path = this->m_settings->get_string("dosbox-path"),
                   command          = Glib::ustring::compose("%1 -conf \"%2\"", dosbox_exec_path, config_file);

    Glib::spawn_command_line_async(command);
}

/**
 * Execute the setup program of the selected game with DOSBox.
 */
void MainWindow::on_setup_activated()
{
    Glib::ustring  id               = this->get_selected_ids()[0],
                   basedir          = this->m_settings->get_string("profiles-path"),
                   basename         = Glib::ustring::compose("%1_setup.conf", id),
                   config_file      = Glib::build_filename(basedir, basename),
                   dosbox_exec_path = this->m_settings->get_string("dosbox-path"),
                   command          = Glib::ustring::compose("%1 -conf \"%2\"", dosbox_exec_path, config_file);

    Glib::spawn_command_line_async(command);
}

/**
 * Edit the application's preferences.
 */
void MainWindow::on_preferences_activated()
{
    auto resource_path = Glib::ustring::compose("%1gui/preferencesdialog.glade", APP_PATH);
    auto builder = Gtk::Builder::create_from_resource(resource_path);
    PreferencesDialog *dialog = nullptr;

    builder->get_widget_derived("PreferencesDialog", dialog);
    dialog->set_transient_for(*this);

    if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
        dialog->apply_settings();
    }
}

/**
 * Show the application's about dialog.
 */
void MainWindow::on_about_activated()
{
    Gtk::AboutDialog dialog;
    Tools::ResourceManager res_man(APP_PATH);
    auto close_button = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_CANCEL));

    close_button->set_image_from_icon_name("window-close");
    dialog.set_position(Gtk::WIN_POS_CENTER);
    dialog.set_icon_name("help-about");
    dialog.set_logo(res_man.get_image("icons/dosboxgtk.svg")->scale_simple(128, 128, Gdk::INTERP_HYPER));
    dialog.set_program_name(PROJECT_NAME);
    dialog.set_version(Glib::ustring::compose("%1.%2.%3.%4", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TWEAK));
    dialog.set_license_type(Gtk::LICENSE_GPL_3_0);
    dialog.set_authors({"Javier Campón Pichardo"});
    dialog.set_website_label(Glib::ustring::compose(_("%1 GitHub respository"), PROJECT_NAME));
    dialog.set_website("https://github.com/Pitxi/DOSBoxGtk");
    dialog.set_comments(Glib::ustring::compose(_("%1 is a GUI frontend for DOSBox developed in C++\n"
                                                 "using GTKmm, LibXML++ and cURLpp libraries.\n\n"
                                                 "Thanks:\n\n"
                                                 "To the people behind DOSBox\n"
                                                 "for the countless improductive hours i spent playing oldies XD\n\n\n"), PROJECT_NAME));

    dialog.run();
}

/**
 * Quit the DOSBoxGTK.
 */
void MainWindow::on_quit_activated()
{
    this->get_application()->quit();
}

/**
 * Monitors changes in the profiles XML file.
 * @param file A file.
 * @param other_file A file or 0.
 * @param event_type Event type.
 */
void MainWindow::on_profiles_file_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    if (event_type == Gio::FILE_MONITOR_EVENT_DELETED || Gio::FILE_MONITOR_EVENT_MOVED) {
        this->create_profiles_file();
    }
}

/**
 * Constructor.
 * @param cobject Underlying C object for the Base Class constructor.
 * @param builder Gtk::Builder used to retrieve the child widgets.
 */
MainWindow::MainWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) :
    Gtk::ApplicationWindow(cobject)
{
    builder->set_translation_domain(PACKAGE);
    builder->get_widget("ProfilesTV", this->m_profiles_tv);

    this->m_settings = Gio::Settings::create(APP_ID, APP_PATH);
    Tools::ResourceManager res_man(APP_PATH);

    auto icon_theme = Gtk::IconTheme::create();

    icon_theme->add_builtin_icon("dosboxgtk-add", 48, res_man.get_image("icons/add.png"));
    icon_theme->add_builtin_icon("dosboxgtk-edit", 48, res_man.get_image("icons/edit.png"));
    icon_theme->add_builtin_icon("dosboxgtk-remove", 48, res_man.get_image("icons/remove.png"));
    icon_theme->add_builtin_icon("dosboxgtk-run", 48, res_man.get_image("icons/run.png"));
    icon_theme->add_builtin_icon("dosboxgtk-run_setup", 48, res_man.get_image("icons/run_setup.png"));
    icon_theme->add_builtin_icon("dosboxgtk-preferences", 48, res_man.get_image("icons/preferences.png"));
    icon_theme->add_builtin_icon("dosboxgtk-about", 48, res_man.get_image("icons/about.png"));
    icon_theme->add_builtin_icon("dosboxgtk-quit", 48, res_man.get_image("icons/quit.png"));

    this->m_main_ag = Glib::RefPtr<Gtk::ActionGroup>::cast_static(builder->get_object("MainActionGroup")).operator->();
    auto new_action         = this->m_main_ag->get_action("New"),
         edit_action        = this->m_main_ag->get_action("Edit"),
         remove_action      = this->m_main_ag->get_action("Remove"),
         run_action         = this->m_main_ag->get_action("Run"),
         setup_action       = this->m_main_ag->get_action("Setup"),
         preferences_action = this->m_main_ag->get_action("Preferences"),
         about_action       = this->m_main_ag->get_action("About"),
         quit_action        = this->m_main_ag->get_action("Quit");

    new_action->set_icon_name("dosboxgtk-add");
    edit_action->set_icon_name("dosboxgtk-edit");
    remove_action->set_icon_name("dosboxgtk-remove");
    run_action->set_icon_name("dosboxgtk-run");
    setup_action->set_icon_name("dosboxgtk-run_setup");
    preferences_action->set_icon_name("dosboxgtk-preferences");
    about_action->set_icon_name("dosboxgtk-about");
    quit_action->set_icon_name("dosboxgtk-quit");

    this->m_profiles_file = Gio::File::create_for_path(Glib::build_filename(this->m_settings->get_string("profiles-path"), PROFILES_FILENAME));
    this->m_profiles_monitor = this->m_profiles_file->monitor_file();

    this->set_title(Glib::ustring::compose("%1 v%2.%3.%4.%5", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TWEAK));

    this->create_profiles_file();

    auto profiles_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_profiles_tv->get_model());
    profiles_ls->set_sort_column(0, Gtk::SORT_ASCENDING);

    about_action->set_label(Glib::ustring::compose(_("About %1..."), PROJECT_NAME));
    about_action->set_tooltip(Glib::ustring::compose(_("Shows information about %1."), PROJECT_NAME));

    // Signals
    this->m_profiles_tv->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_profiles_tv_selection_changed));
    this->m_profiles_tv->signal_row_activated().connect(sigc::mem_fun(*this, &MainWindow::on_row_activated));
    new_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_new_activated));
    edit_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_edit_activated));
    remove_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_remove_activated));
    run_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_run_activated));
    setup_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_setup_activated));
    preferences_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_preferences_activated));
    about_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_about_activated));
    quit_action->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_quit_activated));
    this->m_profiles_monitor->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_profiles_file_changed));

    this->load_profiles();
    this->show_all_children();
}

} // DOSBoxGTK
