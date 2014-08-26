/**
 * @file
 * DOSBoxGTK entry point.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "config.h"
#include "preferencesdialog.h"
#include "mainwindow.h"
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <glibmm/stringutils.h>
#include <gtkmm/application.h>
#include <gtkmm/messagedialog.h>
#include <locale>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Check if the applications settings are ok.
 * @return @c TRUE if the settings are defined or @c FALSE otherwise.
 */
bool check_settings()
{
    auto settings = Gio::Settings::create(APP_ID, APP_PATH);

    auto dosbox_path    = settings->get_string("dosbox-path"),
         default_config = settings->get_string("default-config"),
         profiles_path  = settings->get_string("profiles-path"),
         captures_path  = settings->get_string("captures-path");
    auto dosbox_path_file    = Gio::File::create_for_path(dosbox_path),
         default_config_file = Gio::File::create_for_path(default_config),
         profiles_path_file  = Gio::File::create_for_path(profiles_path),
         captures_path_file  = Gio::File::create_for_path(captures_path);

    return !dosbox_path.empty()    && dosbox_path_file->query_exists()    && (dosbox_path_file->query_file_type()    == Gio::FILE_TYPE_REGULAR)   &&
           !default_config.empty() && default_config_file->query_exists() && (default_config_file->query_file_type() == Gio::FILE_TYPE_REGULAR)   &&
           !profiles_path.empty()  && profiles_path_file->query_exists()  && (profiles_path_file->query_file_type()  == Gio::FILE_TYPE_DIRECTORY) &&
           !captures_path.empty()  && captures_path_file->query_exists()  && (captures_path_file->query_file_type()  == Gio::FILE_TYPE_DIRECTORY);
}

/**
 * Forces the Preferences dialog.
 */
void force_setup()
{
    if (!check_settings()) {
        auto message = Glib::ustring::compose(_("%1 settings are not correctly configured.\n\n"
                                                "This usually happens the first time it is run.\n"
                                                "Now the %1 preferences dialog will open so you can configure the application.\n\n"
                                                "If you do not configure the application correctly it will close.\n"
                                                "Don't forget you need to have DOSBox installed to use %1."), PROJECT_NAME);
        Gtk::MessageDialog msg_dialog(message, false, Gtk::MESSAGE_WARNING);

        msg_dialog.run();
        msg_dialog.hide();
        auto resource_path = Glib::ustring::compose("%1gui/preferencesdialog.glade", APP_PATH);
        auto builder = Gtk::Builder::create_from_resource(resource_path);
        PreferencesDialog *dialog = nullptr;

        builder->get_widget_derived("PreferencesDialog", dialog);

        if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
            dialog->apply_settings();
        }

        if (!check_settings()) {
            auto message = Glib::ustring::compose(_("%1 can not continue without correct settings.\n"
                                                    "The application will close now.\n"), PROJECT_NAME);
            Gtk::MessageDialog msg_dialog(message, false, Gtk::MESSAGE_ERROR);

            msg_dialog.run();
            exit(1);
        }
    }
}

} // DOSBoxGtk

/**
 * Entry point.
 * @param argc Number of arguments.
 * @param argv Arguments array.
 */
int main(int argc, char** argv)
{
    std::locale::global(std::locale(""));
    textdomain(PACKAGE);
    bindtextdomain(PACKAGE, LOCALE_DIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
#ifdef DEBUG
    Glib::setenv("GSETTINGS_SCHEMA_DIR", "./schemas", true);
#endif // DEBUG
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, APP_ID);
    Glib::set_application_name(PROJECT_NAME);

    DOSBoxGTK::force_setup();
    DOSBoxGTK::MainWindow *main_window = nullptr;

    Glib::ustring mainwindow_resource_path = Glib::build_filename(APP_PATH, "gui/mainwindow.glade");
    auto builder = Gtk::Builder::create_from_resource(mainwindow_resource_path);
    builder->get_widget_derived("MainWindow", main_window);

    return app->run(*main_window);
}
