/**
 * @file
 * PreferencesDialog class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "preferencesdialog.h"
#include "config.h"
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <glibmm/spawn.h>
#include <glibmm/regex.h>
#include <giomm/file.h>
#include <gtkmm/grid.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Updates the dialog buttons.
 */
void PreferencesDialog::update_controls()
{
    this->m_dosbox_path_undo_button->set_sensitive   (!this->m_settings_dosbox_path.empty()    && (this->m_settings_dosbox_path    != this->m_dosbox_fcb->get_filename()));
    this->m_default_config_undo_button->set_sensitive(!this->m_settings_default_config.empty() && (this->m_settings_default_config != this->m_default_config_fcb->get_filename()));
    this->m_profiles_path_undo_button->set_sensitive (!this->m_settings_profiles_path.empty()  && (this->m_settings_profiles_path  != this->m_profiles_fcb->get_filename()));
    this->m_captures_path_undo_button->set_sensitive (!this->m_settings_captures_path.empty()  && (this->m_settings_captures_path  != this->m_captures_fcb->get_filename()));
    this->set_response_sensitive(Gtk::RESPONSE_ACCEPT,
                                 !this->m_dosbox_fcb->get_filename().empty() &&
                                 !this->m_default_config_fcb->get_filename().empty() &&
                                 !this->m_profiles_fcb->get_filename().empty() &&
                                 !this->m_captures_fcb->get_filename().empty());
}

/**
 * Creates the DOSBox default config file.
 */
std::string PreferencesDialog::get_default_config()
{
    auto dosbox_path = this->m_dosbox_fcb->get_filename();
    auto dosbox_file = Gio::File::create_for_path(dosbox_path);
    std::string output;

    if (!dosbox_path.empty() && dosbox_file->query_exists()) {
        auto command = Glib::ustring::compose("%1 -printconf", dosbox_path);
        int exit;

        // Get the DOSBox default config path.
        // This will also create the requested file if it's not present.
        Glib::spawn_command_line_sync(command, &output, nullptr, &exit);

        // If the commands execution goes ok...
        if (exit == 0) {
            // Eliminating spaces and new line characters with a regular expression.
            auto regex = Glib::Regex::create("\\n");
            output = regex->replace(output, 0, Glib::ustring(), Glib::REGEX_MATCH_NEWLINE_ANY);

            // Assign the result to the widget.
            this->m_default_config_fcb->set_filename(output);
        }
    }

    return output;
}

/**
 * Initialize controls with values from application settings.
 */
void PreferencesDialog::init()
{
    this->m_settings_dosbox_path    = this->m_settings->get_string("dosbox-path");
    this->m_settings_default_config = this->m_settings->get_string("default-config");
    this->m_settings_profiles_path  = this->m_settings->get_string("profiles-path");
    this->m_settings_captures_path  = this->m_settings->get_string("captures-path");

    auto default_dosbox_path   = Gio::File::create_for_path(Glib::find_program_in_path("dosbox")),
         default_profiles_path = Gio::File::create_for_path(Glib::build_filename(Glib::get_user_data_dir(), PROJECT_NAME, "profiles")),
         default_captures_path = Gio::File::create_for_path(Glib::build_filename(Glib::get_user_data_dir(), PROJECT_NAME, "captures"));

    if (default_dosbox_path->query_file_type() == Gio::FILE_TYPE_REGULAR) {
        this->m_dosbox_fcb->set_file(default_dosbox_path);
    }

    // Creates default profiles folder.
    if (!default_profiles_path->query_exists()) {
        default_profiles_path->make_directory_with_parents();
    }

    // Creates default captures folder.
    if (!default_captures_path->query_exists()) {
        default_captures_path->make_directory_with_parents();
    }

    this->m_profiles_fcb->select_file(default_profiles_path);
    this->m_captures_fcb->select_file(default_captures_path);

    if (!this->m_settings_dosbox_path.empty()) {
        this->m_dosbox_fcb->set_filename(this->m_settings_dosbox_path);
    }

    this->m_default_config_fcb->set_current_folder(Glib::get_home_dir());

    if (default_profiles_path->query_file_type() == Gio::FILE_TYPE_REGULAR) {
        this->m_default_config_fcb->set_filename(this->m_settings_default_config);
    } else {
        auto default_config = this->get_default_config();

        if (!default_config.empty()) {
            this->m_default_config_fcb->set_filename(default_config);
        }
    }

    if (!this->m_settings_profiles_path.empty()) {
        this->m_profiles_fcb->set_filename(this->m_settings_profiles_path);
    }

    if (!this->m_settings_captures_path.empty()) {
        this->m_captures_fcb->set_filename(this->m_settings_captures_path);
    }

    this->update_controls();
}

/**
 * Process response and closes dialog window.
 * @param response_id Dialog response value;
 */
void PreferencesDialog::on_response(int response_id)
{
    Gtk::Dialog::on_response(response_id);
    this->hide();
}

/**
 * Updates the controls when a file or folder is selected in one of the
 * Gtk::FileChooserButton objects.
 */
void PreferencesDialog::on_fcb_file_set()
{
    this->update_controls();
}

/**
 * Undo changes on DOSBox executable path.
 */
void PreferencesDialog::on_dosbox_path_undo_button_clicked()
{
    this->m_dosbox_fcb->set_filename(this->m_settings_dosbox_path);
    this->update_controls();
}

/**
 * Undo changes on profiles path.
 */
void PreferencesDialog::on_profiles_path_undo_button_clicked()
{
    this->m_profiles_fcb->set_filename(this->m_settings_profiles_path);
    this->update_controls();
}

/**
 * Undo changes on captures path.
 */
void PreferencesDialog::on_captures_path_undo_button_clicked()
{
    this->m_captures_fcb->set_filename(this->m_settings_captures_path);
    this->update_controls();
}

/**
 * Constructor.
 * @param cobject Underlying C object for the Base Class constructor.
 * @param builder Gtk::Builder used to retrieve the child widgets.
 */
PreferencesDialog::PreferencesDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) :
    Gtk::Dialog(cobject)
{
    builder->set_translation_domain(PACKAGE);

    Gtk::Grid *content_grid = nullptr;

    auto dosbox_filter = Gtk::FileFilter::create(),
         exe_filter    = Gtk::FileFilter::create(),
         conf_filter   = Gtk::FileFilter::create(),
         all_filter    = Gtk::FileFilter::create();
    this->m_settings   = Gio::Settings::create(APP_ID, APP_PATH);

    builder->get_widget("DOSBoxFCB",               this->m_dosbox_fcb);
    builder->get_widget("DefaultConfigFCB",        this->m_default_config_fcb);
    builder->get_widget("ProfilesFCB",             this->m_profiles_fcb);
    builder->get_widget("CapturesFCB",             this->m_captures_fcb);
    builder->get_widget("CancelButton",            this->m_cancel_button);
    builder->get_widget("AcceptButton",            this->m_accept_button);
    builder->get_widget("DOSBoxPathUndoButton",    this->m_dosbox_path_undo_button);
    builder->get_widget("DefaultConfigUndoButton", this->m_default_config_undo_button);
    builder->get_widget("ProfilesPathUndoButton",  this->m_profiles_path_undo_button);
    builder->get_widget("CapturesPathUndoButton",  this->m_captures_path_undo_button);
    builder->get_widget("ContentGrid",             content_grid);

    this->m_settings->delay();

    dosbox_filter->set_name(_("DOSBox executable"));
    dosbox_filter->add_pattern("[dD][oO][sD[bB][oO][xX]");
    dosbox_filter->add_pattern("[dD][oO][sD[bB][oO][xX].[eE][xX][eE]");

    exe_filter->set_name(_("Executable files"));
    exe_filter->add_mime_type("application/x-executable");

    conf_filter->set_name(_("DOSBox configuration files"));
    conf_filter->add_mime_type("text/plain");
    conf_filter->add_pattern("*.[cC][oO][nN][fF]");

    all_filter->set_name(_("All files"));
    all_filter->add_pattern("*");

    this->m_dosbox_fcb->add_filter(dosbox_filter);
    this->m_dosbox_fcb->add_filter(exe_filter);
    this->m_dosbox_fcb->add_filter(all_filter);

    this->m_default_config_fcb->add_filter(conf_filter);
    this->m_default_config_fcb->add_filter(all_filter);

    this->init();

    for (auto child : content_grid->get_children()) {
        auto fcb = dynamic_cast<Gtk::FileChooserButton*>(child);
        if (fcb != nullptr) {
            fcb->signal_file_set().connect(sigc::mem_fun(*this, &PreferencesDialog::on_fcb_file_set));
        }
    }

    this->m_dosbox_path_undo_button->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_dosbox_path_undo_button_clicked));
    this->m_profiles_path_undo_button->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_profiles_path_undo_button_clicked));
    this->m_captures_path_undo_button->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_captures_path_undo_button_clicked));
}

/**
 * Apply changes to application settings.
 */
void PreferencesDialog::apply_settings() const
{
    this->m_settings->set_string("dosbox-path",    this->m_dosbox_fcb->get_filename());
    this->m_settings->set_string("default-config", this->m_default_config_fcb->get_filename());
    this->m_settings->set_string("profiles-path",  this->m_profiles_fcb->get_filename());
    this->m_settings->set_string("captures-path",  this->m_captures_fcb->get_filename());

    this->m_settings->apply();
}

} // DOSBoxGTK
