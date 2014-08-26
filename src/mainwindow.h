/**
 * @file
 * MainWindow class definition.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/treeview.h>
#include <gtkmm/actiongroup.h>
#include <giomm/settings.h>
#include <libxml++/parsers/domparser.h>


/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * DOSBoxGTK main window.
 */
class MainWindow final : public Gtk::ApplicationWindow
{
private:
    Gtk::ActionGroup *m_main_ag  = nullptr;
    Gtk::TreeView *m_profiles_tv = nullptr;

    Glib::RefPtr<Gio::Settings> m_settings; ///< Application's settings manager.
    Glib::RefPtr<Gio::File> m_profiles_file;
    Glib::RefPtr<Gio::FileMonitor> m_profiles_monitor;
    bool check_settings() const;
    void force_setup();
    xmlpp::DomParser m_parser;

    void create_profiles_file();
    void load_profiles();
    std::vector<Glib::ustring> get_selected_ids() const;
    xmlpp::Element *get_profile_element(const Glib::ustring &id) const;
    bool remove_profile(const Glib::ustring &id);

protected:
    void on_profiles_tv_selection_changed();
    void on_row_activated(const Gtk::TreePath &path, Gtk::TreeViewColumn *column);
    void on_new_activated();
    void on_edit_activated();
    void on_remove_activated();
    void on_run_activated();
    void on_setup_activated();
    void on_preferences_activated();
    void on_about_activated();
    void on_quit_activated();
    void on_profiles_file_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);

public:
    MainWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder);
    virtual ~MainWindow() {}
};

} // DOSBoxGTK

#endif // MAINWINDOW_H
