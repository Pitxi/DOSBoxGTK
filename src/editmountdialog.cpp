/**
 * @file
 * EditMountDialog class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "editmountdialog.h"
#include "config.h"
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <glibmm/stringutils.h>
#include <glibmm/regex.h>
#include <gtkmm/filechooserdialog.h>
#include <iostream>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Validates the dialog controls.
 */
void EditMountDialog::validate_controls()
{
    auto folder = this->m_mount_dir_fcb->get_file();
    this->m_accept_button->set_sensitive(folder && folder->query_exists() && folder->query_file_type() == Gio::FILE_TYPE_DIRECTORY);

    if (!this->m_dir_mount_rb->get_active()) {
        this->m_accept_button->set_sensitive(this->m_images_ls->children().size() > 0);
    }
}

/**
 * Process response and closes dialog window.
 * @param response_id Dialog response value.
 */
void EditMountDialog::on_response(int response_id)
{
    Gtk::Dialog::on_response(response_id);
    this->hide();
}

/**
 * Sets the sensitivity of the related widgets acoording to the active property
 * of the RadioButton.
 */
void EditMountDialog::on_dir_mount_rb_toggled()
{
    this->m_dir_mount_grid->set_sensitive(this->m_dir_mount_rb->get_active());
    this->m_image_mount_grid->set_sensitive(!this->m_dir_mount_rb->get_active());
    this->validate_controls();
}

/**
 * Sets the sensitivity of the controls according to the optioen selected in the
 * ComboBox.
 */
void EditMountDialog::on_mount_dir_type_cbt_changed()
{
    Glib::ustring option = this->m_mount_dir_type_cbt->get_active_id();
    bool is_dir    = option == "dir",
         is_cdrom  = option == "cdrom",
         is_floppy = option == "floppy";

    this->m_usecd_cb->set_sensitive(is_cdrom);
    this->m_usecd_sb->set_sensitive(is_cdrom && this->m_usecd_cb->get_active());
    this->m_cd_access_cbt->set_sensitive(is_cdrom);
    this->m_freesize_cb->set_sensitive(is_dir || is_floppy);
    this->m_freesize_sb->set_sensitive((is_dir || is_floppy) && this->m_freesize_cb->get_active());
    this->m_unit_label->set_text("Mb");

    if (is_floppy) {
        this->m_unit_label->set_text("Kb");
    }
}

/**
 * Toggles the sensitivity and values of the related widgets when the SDL CD-ROM
 * support CheckBox does.
 */
void EditMountDialog::on_usecd_cb_toggled()
{
    this->m_cd_access_cbt->set_sensitive();
    this->m_usecd_sb->set_sensitive(false);

    if (this->m_usecd_cb->get_active()) {
        this->m_cd_access_cbt->set_active_id("noioctl");
        this->m_cd_access_cbt->set_sensitive(false);
        this->m_usecd_sb->set_sensitive();
    }
}

/**
 * Toggles the sensitivity and values of the related widgets when the Freesize
 * command CheckBox does.
 */
void EditMountDialog::on_freesize_cb_toggled()
{
    this->m_freesize_sb->set_sensitive(this->m_freesize_cb->get_active());
}

/**
 * Sets the sensitivity of the mounting images ToolButtons according to the
 * number of selected images in the TreeView
 */
void EditMountDialog::on_images_tv_selection_changed()
{
    int n_rows = this->m_images_tv->get_selection()->count_selected_rows();

    this->m_remove_image_tb->set_sensitive(n_rows > 0);
}

/**
 * Opens a file chooser dialog to choose the images to be mounted.
 */
void EditMountDialog::on_add_image_tb_clicked()
{
    Gtk::FileChooserDialog dialog(*this, _("Select image files for mounting"), Gtk::FILE_CHOOSER_ACTION_OPEN);
    auto all_filter = Gtk::FileFilter::create(),
         cue_filter = Gtk::FileFilter::create(),
         img_filter = Gtk::FileFilter::create();

    all_filter->set_name(_("All files"));
    all_filter->add_pattern("*");

    cue_filter->set_name(_("CUE sheets files"));
    cue_filter->add_mime_type("application/x-cue");
    cue_filter->add_pattern("*.[cC][uU][eE]");

    img_filter->set_name(_("Image files"));
    img_filter->add_mime_type("application/x-raw-disk-image");
    img_filter->add_mime_type("application/x-cd-image");
    img_filter->add_pattern("*.[iI][sS][oO]");
    img_filter->add_pattern("*.[iI][mM][gG]");

    dialog.add_filter(cue_filter);
    dialog.add_filter(img_filter);
    dialog.add_filter(all_filter);

    dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL)->set_image_from_icon_name("dialog-cancel");
    dialog.add_button(_("Accept"), Gtk::RESPONSE_ACCEPT)->set_image_from_icon_name("dialog-ok");
    dialog.set_select_multiple();
    dialog.set_current_folder(this->m_last_folder);

    if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
        for (auto filename : dialog.get_filenames()) {
            auto iter = this->m_images_ls->children().begin(),
                 end = this->m_images_ls->children().end();

            // Check if the image file is aledy in the TreeView.
            while (iter != end) {
                Glib::ustring value;

                iter->get_value(0, value);
                if (value == filename) {
                    break;
                }
                ++iter;
            }

            // If the image file is not in the TreeView it is added.
            if (iter == end) {
                this->m_images_ls->append()->set_value(0, filename);
                this->m_last_folder = dialog.get_current_folder();
            }
        }
    }

    this->validate_controls();
}

/**
 * Remove the selected images when the remove ToolButton is clicked.
 */
void EditMountDialog::on_remove_image_tb_clicked()
{
    auto paths = this->m_images_tv->get_selection()->get_selected_rows();
    std::vector<Gtk::TreeIter> iters;

    for (auto path : paths) {
        iters.push_back(this->m_images_ls->get_iter(path));
    }

    for (auto iter : iters) {
        this->m_images_ls->erase(iter);
    }
}

/**
 * Constructor.
 * @param object Underlying C object for the Base Class constructor.
 * @param builder Gtk::Builder used to retrieve the child widgets.
 */
EditMountDialog::EditMountDialog(BaseObjectType *object, const Glib::RefPtr<Gtk::Builder> &builder) :
    Gtk::Dialog(object), m_builder(builder)
{
    this->m_builder->set_translation_domain(PACKAGE);

    this->m_builder->get_widget("DirMountGrid",    this->m_dir_mount_grid);
    this->m_builder->get_widget("ImageMountGrid",  this->m_image_mount_grid);
    this->m_builder->get_widget("DriveLetterCBT",  this->m_drive_letter_cbt);
    this->m_builder->get_widget("MountDirTypeCBT", this->m_mount_dir_type_cbt);
    this->m_builder->get_widget("CDAccessCBT",     this->m_cd_access_cbt);
    this->m_builder->get_widget("ImageTypeCBT",    this->m_image_type_cbt);
    this->m_builder->get_widget("DirMountRB",      this->m_dir_mount_rb);
    this->m_builder->get_widget("ImageMountRB",    this->m_image_mount_rb);
    this->m_builder->get_widget("UsecdCB",         this->m_usecd_cb);
    this->m_builder->get_widget("FreesizeCB",      this->m_freesize_cb);
    this->m_builder->get_widget("UsecdSB",         this->m_usecd_sb);
    this->m_builder->get_widget("FreesizeSB",      this->m_freesize_sb);
    this->m_builder->get_widget("LabelEntry",      this->m_label_entry);
    this->m_builder->get_widget("UnitLabel",       this->m_unit_label);
    this->m_builder->get_widget("ImagesTV",        this->m_images_tv);
    this->m_builder->get_widget("AddImageTB",      this->m_add_image_tb);
    this->m_builder->get_widget("RemoveImageTB",   this->m_remove_image_tb);
    this->m_builder->get_widget("MountDirFCB",     this->m_mount_dir_fcb);
    this->m_builder->get_widget("AcceptButton",    this->m_accept_button);

    this->m_images_ls   = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(this->m_builder->get_object("ImagesLS"));
    this->m_last_folder = Glib::get_home_dir();

    // signals
    this->m_dir_mount_rb->signal_toggled().connect(sigc::mem_fun(*this, &EditMountDialog::on_dir_mount_rb_toggled));
    this->m_mount_dir_type_cbt->signal_changed().connect(sigc::mem_fun(*this, &EditMountDialog::on_mount_dir_type_cbt_changed));
    this->m_usecd_cb->signal_toggled().connect(sigc::mem_fun(*this, &EditMountDialog::on_usecd_cb_toggled));
    this->m_freesize_cb->signal_toggled().connect(sigc::mem_fun(*this, &EditMountDialog::on_freesize_cb_toggled));
    this->m_images_tv->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &EditMountDialog::on_images_tv_selection_changed));
    this->m_add_image_tb->signal_clicked().connect(sigc::mem_fun(*this, &EditMountDialog::on_add_image_tb_clicked));
    this->m_remove_image_tb->signal_clicked().connect(sigc::mem_fun(*this, &EditMountDialog::on_remove_image_tb_clicked));
    this->m_mount_dir_fcb->signal_file_set().connect(sigc::mem_fun(*this, &EditMountDialog::validate_controls));

    this->set_drive_letters();
}

/**
 * Set the drive letters that canbe used in the mounting point.
 * @param used_letters String with the alredy used letters.
 */
void EditMountDialog::set_drive_letters(const Glib::ustring &used_letters)
{
    int pos = 0;

    this->m_drive_letter_cbt->remove_all();

    for (char l = 'A'; l < 'Z'; ++l) {
        if (used_letters.uppercase().find(l) == Glib::ustring::npos) {
            Glib::ustring letter_str(1, l);
            this->m_drive_letter_cbt->insert(pos, letter_str, letter_str);
            ++pos;
        }
    }

    this->m_drive_letter_cbt->set_wrap_width(6);
    this->m_drive_letter_cbt->set_active(0);

    if (used_letters.uppercase().find('C') == Glib::ustring::npos) {
        this->m_drive_letter_cbt->set_active_id("C");
    }
}

/**
 * Gets the resulting mounting command from the dialog.
 * @return DOSBox mount command.
 */
Glib::ustring EditMountDialog::get_command() const
{
    Glib::ustring command;
    char letter = *this->m_drive_letter_cbt->get_active_id().begin();

    if (this->m_dir_mount_rb->get_active()) {
        Glib::ustring host_dir = this->m_mount_dir_fcb->get_filename(),
                      type     = this->m_mount_dir_type_cbt->get_active_id(),
                      label    = this->m_label_entry->get_text(),
                      cd_access;
        int usecd = -1,
            freesize = -1;

        if (this->m_cd_access_cbt->get_sensitive()) {
            cd_access = this->m_cd_access_cbt->get_active_id();
        }

        if (this->m_usecd_cb->get_active()) {
            usecd = this->m_usecd_sb->get_value_as_int();
        }

        if (this->m_freesize_cb->get_active()) {
            freesize = this->m_freesize_sb->get_value_as_int();
        }

        command = MountCommand(letter, host_dir, type, label, cd_access, usecd, freesize).get_command();
    } else {
        auto image_type = this->m_image_type_cbt->get_active_id();
        std::vector<Glib::ustring> images;

        for (auto row : this->m_images_tv->get_model()->children()) {
            Glib::ustring image;

            row->get_value(0, image);
            images.push_back(image);
        }

        command = ImgmountCommand(letter, images, image_type).get_command();
    }

    return command;
}

/**
 * Gets the dialog controls values from a DOSBox Mounting command.
 * @param command DOSBox mounting command.
 */
void EditMountDialog::set_command(const MountCommandBase *command)
{
    auto m_command   = dynamic_cast<const MountCommand*>(command);
    auto im_command  = dynamic_cast<const ImgmountCommand*>(command);

    this->m_drive_letter_cbt->set_active_id(Glib::ustring(1, command->get_letter()));

    if (m_command != nullptr) {
        this->m_dir_mount_rb->set_active();
        this->m_mount_dir_type_cbt->set_active_id(m_command->get_type());
        this->m_mount_dir_fcb->set_filename(m_command->get_host_dir());
        this->m_label_entry->set_text(m_command->get_label());
        this->m_usecd_cb->set_active(m_command->get_usecd() > -1);

        if (this->m_usecd_cb->get_active()) {
            this->m_usecd_sb->set_value(m_command->get_usecd());
        }

        this->m_cd_access_cbt->set_active_id(m_command->get_cd_access());
        this->m_freesize_cb->set_active(m_command->get_freesize() > -1);

        if (this->m_freesize_cb->get_active()) {
            this->m_freesize_sb->set_value(m_command->get_freesize());
        }
    }

    if (im_command != nullptr) {
        this->m_image_mount_rb->set_active();
        this->m_image_type_cbt->set_active_id(im_command->get_image_type());
        for (auto image : im_command->get_images()) {
            auto iter = this->m_images_ls->append();

            iter->set_value(0, image);
        }
    }

    this->validate_controls();
}

/**
 * Gets the dialog controls values from a DOSBox Mounting command.
 * @param command String with the DOSBox mounting command.
 */
void EditMountDialog::set_command(const Glib::ustring &command)
{
    auto mounting_command_regex = Glib::Regex::create("^(?'command'MOUNT|IMGMOUNT)", Glib::REGEX_CASELESS);
    Glib::MatchInfo minfo;
    MountCommandBase *mounting_command = nullptr;

    if (mounting_command_regex->match(command, 0, minfo)) {
        if (minfo.fetch_named("command").uppercase() == "MOUNT") {
            mounting_command = new MountCommand(command);
        } else {
            mounting_command = new ImgmountCommand(command);
        }

        this->set_command(mounting_command);
    }

    delete mounting_command;
}

} // DOSBoxGTK
