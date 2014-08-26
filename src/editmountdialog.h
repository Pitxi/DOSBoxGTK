/**
 * @file
 * EditMountDialog class declaration.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef EDITMOUNTDIALOG_H
#define EDITMOUNTDIALOG_H

#include "mountcommand.h"
#include "imgmountcommand.h"
#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/liststore.h>
#include <gtkmm/grid.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/filechooserbutton.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * This Dialog allow to add and edit DOSBox mounting points.
 */
class EditMountDialog : public Gtk::Dialog
{
private:
    Gtk::Grid *m_dir_mount_grid             = nullptr,
              *m_image_mount_grid           = nullptr;
    Gtk::ComboBoxText *m_drive_letter_cbt   = nullptr,
                      *m_mount_dir_type_cbt = nullptr,
                      *m_cd_access_cbt      = nullptr,
                      *m_image_type_cbt     = nullptr;
    Gtk::RadioButton *m_dir_mount_rb        = nullptr,
                     *m_image_mount_rb      = nullptr;
    Gtk::CheckButton *m_usecd_cb            = nullptr,
                     *m_freesize_cb         = nullptr;
    Gtk::SpinButton *m_usecd_sb             = nullptr,
                    *m_freesize_sb          = nullptr;
    Gtk::Entry *m_label_entry               = nullptr;
    Gtk::Label *m_unit_label                = nullptr;
    Gtk::TreeView *m_images_tv              = nullptr;
    Gtk::ToolButton *m_add_image_tb         = nullptr,
                    *m_remove_image_tb      = nullptr;
    Gtk::FileChooserButton *m_mount_dir_fcb = nullptr;
    Gtk::Button *m_accept_button            = nullptr;

    Glib::RefPtr<Gtk::Builder> m_builder;
    Glib::RefPtr<Gtk::ListStore> m_images_ls;
    Glib::ustring m_last_folder;

    void validate_controls();

protected:
    virtual void on_response(int response_id);
    void on_dir_mount_rb_toggled();
    void on_mount_dir_type_cbt_changed();
    void on_usecd_cb_toggled();
    void on_freesize_cb_toggled();
    void on_image_type_cbt_changed();
    void on_images_tv_selection_changed();
    void on_add_image_tb_clicked();
    void on_remove_image_tb_clicked();

public:
    EditMountDialog(BaseObjectType *object, const Glib::RefPtr<Gtk::Builder> &builder);
    virtual ~EditMountDialog() {}

    void set_drive_letters(const Glib::ustring &used_letters = Glib::ustring());
    Glib::ustring get_command() const;
    void set_command(const MountCommandBase *command);
    void set_command(const Glib::ustring &command);
};

} // DOSBoxGTK

#endif // EDITMOUNTDIALOG_H
