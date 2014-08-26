/**
 * @file
 * PreferencesDialog class definition.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserbutton.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Clase PreferencesDialog
 */
class PreferencesDialog final: public Gtk::Dialog
{
private:
    Glib::RefPtr<Gio::Settings> m_settings;
    Glib::ustring m_settings_dosbox_path, m_settings_default_config, m_settings_profiles_path, m_settings_captures_path;
    Gtk::Button *m_cancel_button, *m_accept_button, *m_dosbox_path_undo_button, *m_default_config_undo_button, *m_profiles_path_undo_button, *m_captures_path_undo_button;
    Gtk::FileChooserButton *m_dosbox_fcb, *m_default_config_fcb, *m_profiles_fcb, *m_captures_fcb;

    void update_controls();
    std::string get_default_config();
    void init();

protected:
    virtual void on_response(int response_id) override;
    void on_fcb_file_set();
    void on_dosbox_path_undo_button_clicked();
    void on_profiles_path_undo_button_clicked();
    void on_captures_path_undo_button_clicked();

public:
    PreferencesDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder);
    virtual ~PreferencesDialog() {}
    void apply_settings() const;
};

} // DOSBoxGTK

#endif // PREFERENCESDIALOG_H
