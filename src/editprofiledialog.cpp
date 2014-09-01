/**
 * @file
 * EditProfileDialog class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "mixerdialog.h"
#include "editprofiledialog.h"
#include "editmountdialog.h"
#include "selectgameinfodialog.h"
#include "config.h"
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <glibmm/stringutils.h>
#include <glibmm/fileutils.h>
#include <gtkmm/cssprovider.h>
#include <libxml++/parsers/domparser.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <cpptools/htmltools.hpp>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Process response and closes dialog window.
 * @param response_id Dialog response value;
 */
void EditProfileDialog::on_response(int response_id)
{
    Gtk::Dialog::on_response(response_id);
    this->hide();
}

/**
 * Handler for the entry's changed signal.
 * @param sender The Entry emitting the signal.
 */
void EditProfileDialog::on_entry_changed(Gtk::Entry *sender)
{
    auto text     = sender->get_text();
    auto is_empty = text.empty();

    sender->set_icon_sensitive(Gtk::ENTRY_ICON_SECONDARY, !is_empty);

    if (sender == this->m_title_entry) {
        sender->get_style_context()->add_class("invalid");
        sender->set_icon_from_icon_name("dialog-warning");
        this->m_consult_button->set_sensitive(!is_empty);

        if (!is_empty) {
            sender->get_style_context()->remove_class("invalid");
            sender->set_icon_from_icon_name(Glib::ustring());
        }

        this->validate_controls();
    } else if (sender == this->m_program_entry) {
        sender->get_style_context()->add_class("invalid");

        if (this->m_booter_rb->get_active() || this->check_program(text)) {
            sender->get_style_context()->remove_class("invalid");
        }

        this->validate_controls();
    } else if (sender == this->m_setup_entry) {
        this->m_setup_entry->get_style_context()->add_class("invalid");

        if (this->m_booter_rb->get_active() || is_empty || this->check_program(text)) {
            sender->get_style_context()->remove_class("invalid");
        }

        this->validate_controls();
    }
}

/**
 * Handler for the entry's icon_release signal.
 * @param icon_pos The position of the clicked icon.
 * @param event The button release event.
 * @param sender The Entry emitting the signal.
 */
void EditProfileDialog::on_entry_icon_release(Gtk::EntryIconPosition icon_pos, const GdkEventButton *event, Gtk::Entry *sender)
{
    if (event->button == GDK_BUTTON_PRIMARY) {
        if (icon_pos == Gtk::ENTRY_ICON_PRIMARY) {
            Gtk::FileChooserDialog dialog(*this, Glib::ustring());

            dialog.set_modal();
            dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
            dialog.add_button(_("Accept"), Gtk::RESPONSE_ACCEPT);

            if (sender == this->m_program_entry || sender == this->m_setup_entry) {
                auto exec_filter = Gtk::FileFilter::create();
                auto selection = this->m_mounting_overview_tree_view->get_selection();

                exec_filter->set_name(_("DOS executables"));
                exec_filter->add_pattern("*.[Ee][Xx][Ee]");
                exec_filter->add_pattern("*.[Cc][Oo][Mm]");
                exec_filter->add_pattern("*.[Bb][Aa][Tt]");

                dialog.set_title(_("Select a DOSBox executable..."));
                dialog.add_filter(exec_filter);

                dialog.set_current_folder(Glib::get_home_dir());

                if (!sender->get_text().empty()){
                    dialog.set_filename(sender->get_text());
                }

                if (sender == this->m_setup_entry && dialog.get_filename().empty()) {
                    auto program_filename = this->m_program_entry->get_text();

                    if (!program_filename.empty()) {
                        auto dirname = Glib::path_get_dirname(program_filename);

                        dialog.set_current_folder(dirname);
                    }
                } else if (dialog.get_filename().empty() && selection->count_selected_rows() == 1) {
                    auto iter =this->m_mounting_overview_tree_view->get_model()->get_iter(selection->get_selected_rows()[0]);
                    Glib::ustring command;
                    Glib::MatchInfo minfo;

                    iter->get_value(0, command);
                    this->parse_line(command, PCRE_MOUNT, minfo);

                    if (minfo.fetch_named("command").uppercase() == "MOUNT") {
                        MountCommand m_command(command);
                        dialog.set_current_folder(m_command.get_host_dir());
                    }
                }

                if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
                    sender->set_text(dialog.get_filename());
                }
            } else if (sender == this->m_mapper_file_entry) {
                auto map_filter = Gtk::FileFilter::create(),
                     txt_filter = Gtk::FileFilter::create();

                map_filter->set_name(_("DOSBox map files"));
                map_filter->add_pattern("*.[Mm][Aa][Pp]");

                txt_filter->set_name(_("Text files"));
                txt_filter->add_mime_type("text/plain");
                txt_filter->add_pattern("*.[Tt][Xx][Tt]");

                dialog.add_filter(map_filter);
                dialog.add_filter(txt_filter);

                dialog.set_action(Gtk::FILE_CHOOSER_ACTION_SAVE);
                dialog.set_title(_("Select a DOSBox mapper file for loading/saving..."));
                dialog.add_shortcut_folder(Glib::build_filename(Glib::get_user_data_dir(), PROJECT_NAME));
                dialog.set_filename(sender->get_text());
                dialog.set_current_name(sender->get_text());

                if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
                    sender->set_text(dialog.get_filename());
                }
            }
        } else if (icon_pos == Gtk::ENTRY_ICON_SECONDARY) {
            sender->set_text(Glib::ustring());
        }
    }
}

/**
 * Sets the validation values of the controls when the mounting type changes
 * between program and booter.
 */
void EditProfileDialog::on_program_rb_toggled()
{
    this->m_program_grid->set_sensitive(this->m_program_rb->get_active());
    this->m_booter_grid->set_sensitive(this->m_booter_rb->get_active());
    this->on_entry_changed(this->m_program_entry);
    this->on_entry_changed(this->m_setup_entry);
}

/**
 * Toggles the sensitivity of the loadfix SpinButton when the loadfix
 * CheckButton emits the toggled signal.
 */
void EditProfileDialog::on_loadfix_cb_toggled()
{
    this->m_loadfix_spin_button->set_sensitive(false);
    this->m_loadfix_spin_button->set_value(0);

    if (this->m_loadfix_cb->get_active()) {
        this->m_loadfix_spin_button->set_sensitive(this->m_loadfix_cb->get_active());
        this->m_loadfix_spin_button->set_value(64);
    }
}

/**
 * Opens an EditMountDialog instance for adding.
 */
void EditProfileDialog::on_add_mount_tb_clicked()
{
    EditMountDialog *dialog;
    auto resource_path = Glib::build_filename(APP_PATH, "gui", "editmountdialog.glade");
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_resource(resource_path);

    builder->get_widget_derived("EditMountDialog", dialog);
    dialog->set_drive_letters(this->get_used_letters());

    if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
        this->add_mounting_command(dialog->get_command());
    }

    this->on_entry_changed(this->m_program_entry);
    this->on_entry_changed(this->m_setup_entry);
}

/**
 * Opens an EditMountDialog instance for editing the selected mounting point.
 */
void EditProfileDialog::on_edit_mount_tb_clicked()
{
    EditMountDialog *dialog;
    auto resource_path = Glib::build_filename(APP_PATH, "gui", "editmountdialog.glade");
    auto builder = Gtk::Builder::create_from_resource(resource_path);
    auto model = this->m_mounting_overview_tree_view->get_model();
    auto selected_row_path = this->m_mounting_overview_tree_view->get_selection()->get_selected_rows()[0];
    auto selected_row_iter = model->get_iter(selected_row_path);
    Glib::ustring command;

    builder->get_widget_derived("EditMountDialog", dialog);

    selected_row_iter->get_value(0, command);
    dialog->set_drive_letters(this->get_used_letters(true));
    dialog->set_command(command);

    if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
        selected_row_iter->set_value(0, dialog->get_command());
    }
}

/**
 * Removes the selected mounting points.
 */
void EditProfileDialog::on_remove_mount_tb_clicked()
{
    auto selection = this->m_mounting_overview_tree_view->get_selection();
    auto mounting_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_mounting_overview_tree_view->get_model());
    std::vector<Gtk::TreeIter> selected_rows;

    for (auto path : selection->get_selected_rows()) {
        selected_rows.push_back(mounting_ls->get_iter(path));
    }

    for (auto row : selected_rows) {
        mounting_ls->erase(row);
    }

    this->on_entry_changed(this->m_program_entry);
    this->on_entry_changed(this->m_setup_entry);
}

/**
 * Shows a FileChooserDialog to add images for booting.
 */
void EditProfileDialog::on_add_boot_tb_clicked()
{
    Gtk::FileChooserDialog dialog(*this, _("Select DOSBox boot images..."), Gtk::FILE_CHOOSER_ACTION_OPEN);
    auto img_filter = Gtk::FileFilter::create(),
         all_filter = Gtk::FileFilter::create();

    img_filter->set_name(_("DOSBox boot images"));
    img_filter->add_pattern("*.[iI][mM][gG]");
    img_filter->add_pattern("*.[cC][pP]2");
    img_filter->add_pattern("*.[dD][cC][fF]");
    img_filter->add_pattern("*.[jJ][rR][cC]");
    img_filter->add_pattern("*.[tT][dD]0");

    all_filter->set_name(_("All files"));
    all_filter->add_pattern("*");

    dialog.set_modal();
    dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("Accept"), Gtk::RESPONSE_ACCEPT);
    dialog.set_current_folder(Glib::get_home_dir());
    dialog.set_select_multiple();

    dialog.add_filter(img_filter);
    dialog.add_filter(all_filter);

    if (dialog.run() == Gtk::RESPONSE_ACCEPT) {
        auto booter_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_booter_tree_view->get_model());

        for (auto image : dialog.get_filenames()) {
            auto iter = booter_ls->append();

            iter->set_value(0, image);
        }

        this->validate_controls();
    }
}

/**
 *  Removes the selected boot images.
 */
void EditProfileDialog::on_remove_boot_tb_clicked()
{
    auto selection = this->m_booter_tree_view->get_selection();
    auto booter_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_booter_tree_view->get_model());
    std::vector<Gtk::TreeIter> selected_rows;

    for (auto path : selection->get_selected_rows()) {
        selected_rows.push_back(booter_ls->get_iter(path));
    }

    for (auto row : selected_rows) {
        booter_ls->erase(row);
    }

    this->validate_controls();
}

/**
 * Shows the MixerDialog to change the DOSBox mixer parameters.
 */
void EditProfileDialog::on_mixer_button_clicked()
{
    MixerDialog *dialog;
    auto resource_path = Glib::build_filename(APP_PATH, "gui", "mixerdialog.glade");
    auto builder = Gtk::Builder::create_from_resource(resource_path);

    builder->get_widget_derived("MixerDialog", dialog);

    dialog->parse_command(this->m_mixer_command);

    if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
        this->m_mixer_command = dialog->get_mixer_command();
    }
}

/**
 * Searchs in the MobyGame site for info about the title of the profile.
 */
void EditProfileDialog::on_consult_button_clicked()
{
    SelectGameInfoDialog *dialog;
    auto resource_path = Glib::build_filename(APP_PATH, "gui", "selectgameinfodialog.glade");
    auto builder = Gtk::Builder::create_from_resource(resource_path);

    builder->get_widget_derived("SelectGameInfoDialog", dialog);
    dialog->search_game_info(this->m_title_entry->get_text());

    if (dialog->run() == Gtk::RESPONSE_ACCEPT) {
        curlpp::initialize();
        curlpp::Easy request;
        std::stringstream os;
        auto regex = Glib::Regex::create("niceHeaderTitle\">\\s*<a.+?>(?'title'.+?)\\s*<\\/a>|(?'key'Published by|Developed by|Released|Genre)<\\/div>.+?<a.+?>(?'value'.+?)<\\/a>|<h2>Description<\\/h2>(?'description'.+?)<div");
        Glib::MatchInfo minfo;

        request.setOpt<curlpp::options::Url>(dialog->get_selected_href());
        request.setOpt<curlpp::options::WriteStream>(&os);

        request.perform();
        curlpp::terminate();

        if (regex->match(os.str(), 0, minfo)) {
            this->m_title_entry->set_text(Tools::html_entities_decode(minfo.fetch_named("title")));

            while (minfo.next()) {
                auto br_regex = Glib::Regex::create("<br.*?>.*?(?:<.*?\\/br>)*|<br.*?\\/>", Glib::REGEX_CASELESS),
                     tag_regex = Glib::Regex::create("<(.+?)>");
                auto key   = minfo.fetch_named("key"),
                     value = Tools::html_entities_decode(minfo.fetch_named("value")),
                     description = Tools::html_entities_decode(minfo.fetch_named("description"));

                description = br_regex->replace(description, 0, "\n", static_cast<Glib::RegexMatchFlags>(0)); // Replaces <br> tags with newlines.
                description = tag_regex->replace(description, 0, Glib::ustring(), static_cast<Glib::RegexMatchFlags>(0)); // Removes every html/xml tag from the text.

                if (!description.empty()) {
                    this->m_notes_tv->get_buffer()->set_text(description);
                } else if (key == "Published by") {
                    this->m_publisher_entry->set_text(value);
                } else if (minfo.fetch_named("key") == "Developed by") {
                    this->m_developer_entry->set_text(value);
                } else if (minfo.fetch_named("key") == "Released") {
                    this->m_year_entry->set_text(value.substr(value.size() - 4));
                } else if (minfo.fetch_named("key") == "Genre") {
                    this->m_genre_entry->set_text(value);
                }
            }
        }
    }
}

/**
 * Changes the sensitivity and default value of the cycles SpinButton
 * accordingly to the value of the cycles ComboBoxText when the latter emits the
 * changed signal.
 */
void EditProfileDialog::on_cycles_cbt_changed()
{
    auto value = this->m_cycles_cbt->get_active_id();

    this->m_cycles_spin_button->set_value(0);
    this->m_cycles_spin_button->set_sensitive(false);
    if (value == "fixed") {
        this->m_cycles_spin_button->set_sensitive();
        this->m_cycles_spin_button->set_value(3000);
    }
}

/**
 * Changes the sensitivity of the remove ToolButton related to the given
 * TreeView whose selection has changed.
 * @param tv TreeView whose TreeViewSelection emmitted the signal.
 */
void EditProfileDialog::on_selection_changed(Gtk::TreeView *tv)
{
    auto n = tv->get_selection()->count_selected_rows(); // Number of selected rows in the TreeView.

    if (tv == this->m_mounting_overview_tree_view) {
        this->m_edit_mount_tb->set_sensitive(n == 1);
        this->m_remove_mount_tb->set_sensitive(n > 0);
    } else {
        this->m_remove_boot_tb->set_sensitive(n > 0);
    }
}

/**
 * Parses the given DOSBox configuration file and sets the profile control's
 * values accordingly.
 * @param filename DOSBox configuration file to read from.
 */
void EditProfileDialog::load_config_file(const Glib::ustring &filename)
{
    auto config_parts(this->autoexec_split(filename));
    Glib::KeyFile config;

    config.load_from_data(config_parts[0]);
    if (config_parts.size() > 1) {
        this->parse_autoexec(config_parts[1]);
    }

    // sdl group ---------------------------------------------------------------
    if (config.has_group("sdl")) {
        if (config.has_key("sdl", "fullscreen")) {
            this->m_fullscreen_switch->set_active(config.get_boolean("sdl", "fullscreen"));
        }

        if (config.has_key("sdl", "fulldouble")){
            this->m_double_buffering_switch->set_active(config.get_boolean("sdl", "fulldouble"));
        }

        if (config.has_key("sdl", "fullresolution")) {
            Glib::ustring value = config.get_value("sdl", "fullresolution");

            this->m_full_resolution_cbt->get_entry()->set_text(value);
            if (value == "original") {
                this->m_full_resolution_cbt->set_active_text(value);
            }
        }

        if (config.has_key("sdl", "windowresolution")) {
            Glib::ustring value = config.get_value("sdl", "windowresolution");

            this->m_windowed_resolution_cbt->get_entry()->set_text(value);
            if (value == "original") {
                this->m_windowed_resolution_cbt->set_active_id(value);
            }
        }

        if (config.has_key("sdl", "output")) {
            Glib::ustring value = config.get_value("sdl", "output");
            this->m_outut_cbt->set_active_id(value);
        }

        if (config.has_key("sdl", "autolock")) {
            this->m_mouse_autolock_switch->set_active(config.get_boolean("sdl", "autolock"));
        }

        if (config.has_key("sdl", "sensitivity")) {
            this->m_mouse_sensitivity_spin_button->set_value(config.get_double("sdl", "sensitivity"));
        }

        if (config.has_key("sdl", "waitonerror")) {
            this->m_wait_on_error_switch->set_active(config.get_boolean("sdl", "waitonerror"));
        }

        if (config.has_key("sdl", "priority")) {
            config.set_list_separator(',');
            std::vector<Glib::ustring> values = config.get_string_list("sdl", "priority");

            this->m_priority_active_cbt->set_active_id(values[0]);
            this->m_priority_inactive_cbt->set_active_id(values[1]);
        }

        if (config.has_key("sdl", "mapperfile")) {
            auto mapper_file = config.get_value("sdl", "mapperfile");

            if (!mapper_file.empty()) {
                if (!Glib::file_test(mapper_file, Glib::FILE_TEST_IS_REGULAR)) {
                    mapper_file = Glib::build_filename(Glib::get_user_data_dir(), PROJECT_NAME, Glib::path_get_basename(mapper_file));
                }

                this->m_mapper_file_entry->set_text(mapper_file);
            }
        }

        if (config.has_key("sdl", "usescancodes")) {
            this->m_keyboard_scancodes_switch->set_active(config.get_boolean("sdl", "usescancodes"));
        }
    }

    // dosbox group ------------------------------------------------------------
    if (config.has_group("dosbox")) {
        if (config.has_key("dosbox", "language")) {
            Glib::ustring value = config.get_value("dosbox", "language");

            if (!value.empty()) {
                this->m_language_file_fcb->set_filename(value);
            }
        }

        if (config.has_key("dosbox", "machine")){
            this->m_machine_cbt->set_active_id(config.get_value("dosbox", "machine"));
        }

        if (config.has_key("dosbox", "captures")) {
            Glib::ustring value = config.get_value("dosbox", "captures");

            if (!Glib::file_test(value, Glib::FILE_TEST_IS_DIR)) {
                value = this->m_settings->get_string("captures-path");
            }

            this->m_captures_fcb->set_filename(value);
        }

        if (config.has_key("dosbox", "memsize")) {
            this->m_memory_size_spin_button->set_value(config.get_double("dosbox", "memsize"));
        }
    }

    // render group ------------------------------------------------------------
    if (config.has_group("render")) {
        if (config.has_key("render", "frameskip")) {
            this->m_frame_skip_spin_button->set_value(config.get_double("render", "frameskip"));
        }

        if (config.has_key("render", "aspect")) {
            this->m_aspect_correction_switch->set_active(config.get_boolean("render", "aspect"));
        }

        if (config.has_key("render", "scaler")) {
            config.set_list_separator(' ');
            std::vector<Glib::ustring> values = config.get_string_list("render", "scaler");

            this->m_scaler_cbt->set_active_id(values[0]);
            if (values.size() > 1) {
                this->m_forced_scaler_switch->set_active(values[1] == "forced");
            }
        }
    }

    // cpu group ---------------------------------------------------------------
    if (config.has_group("cpu")) {
        if (config.has_key("cpu", "core")) {
            this->m_core_cbt->set_active_id(config.get_value("cpu", "core"));
        }

        if (config.has_key("cpu", "cputype")) {
            this->m_cpu_type_cbt->set_active_id(config.get_value("cpu", "cputype"));
        }

        if (config.has_key("cpu", "cycles")) {
            config.set_list_separator(' ');
            std::vector<Glib::ustring> values = config.get_string_list("cpu", "cycles");

            this->m_cycles_cbt->set_active_id(values[0]);
            this->m_cycles_spin_button->set_value(0);

            if (values.size() > 1) {
                this->m_cycles_spin_button->set_value(Glib::Ascii::strtod(values[1]));
            }
        }

        if (config.has_key("cpu", "cycleup")) {
            this->m_cycle_up_spin_button->set_value(config.get_double("cpu", "cycleup"));
        }

        if (config.has_key("cpu", "cycledown")) {
            this->m_cycle_down_spin_button->set_value(config.get_double("cpu", "cycledown"));
        }
    }

    // mixer group -------------------------------------------------------------
    if (config.has_group("mixer")) {
        if (config.has_key("mixer", "nosound")) {
            this->m_silent_mode_switch->set_active(config.get_boolean("mixer", "nosound"));
        }

        if (config.has_key("mixer", "rate")) {
            this->m_general_sample_rate_cbt->set_active_id(config.get_value("mixer", "rate"));
        }

        if (config.has_key("mixer", "blocksize")) {
            this->m_block_size_cbt->set_active_id(config.get_value("mixer", "blocksize"));
        }

        if (config.has_key("mixer", "prebuffer")) {
            this->m_prebuffer_spin_button->set_value(config.get_double("mixer", "prebuffer"));
        }
    }

    // midi group --------------------------------------------------------------
    if (config.has_group("midi")) {
        if (config.has_key("midi", "mpu401")) {
            this->m_mpu401_cbt->set_active_id(config.get_value("midi", "mpu401"));
        }

        if (config.has_key("midi", "mididevice")) {
            this->m_midi_device_cbt->set_active_id(config.get_value("midi", "mididevice"));
        }

        if (config.has_key("midi", "midiconfig")) {
            this->m_midi_config_entry->set_text(config.get_value("midi", "midiconfig"));
        }
    }

    // sblaster group ----------------------------------------------------------
    if (config.has_group("sblaster")) {
        if (config.has_key("sblaster", "sbtype")) {
            this->m_sb_type_cbt->set_active_id(config.get_value("sblaster", "sbtype"));
        }

        if (config.has_key("sblaster", "sbbase")) {
            this->m_sb_address_cbt->set_active_id(config.get_value("sblaster", "sbbase"));
        }

        if (config.has_key("sblaster", "irq")) {
            this->m_sb_irq_cbt->set_active_id(config.get_value("sblaster", "irq"));
        }

        if (config.has_key("sblaster", "dma")) {
            this->m_sb_dma_cbt->set_active_id(config.get_value("sblaster", "dma"));
        }

        if (config.has_key("sblaster", "hdma")) {
            this->m_sb_hdma_cbt->set_active_id(config.get_value("sblaster", "hdma"));
        }

        if (config.has_key("sblaster", "sbmixer")) {
            this->m_sb_mixer_switch->set_active(config.get_boolean("sblaster", "sbmixer"));
        }

        if (config.has_key("sblaster", "oplmode")) {
            this->m_sb_opl_mode_cbt->set_active_id(config.get_value("sblaster", "oplmode"));
        }

        if (config.has_key("sblaster", "oplemu")) {
            this->m_sb_opl_emulation_cbt->set_active_id(config.get_value("sblaster", "oplemu"));
        }

        if (config.has_key("sblaster", "oplrate")) {
            this->m_sb_sample_rate_cbt->set_active_id(config.get_value("sblaster", "oplrate"));
        }
    }

    // gus group ---------------------------------------------------------------
    if (config.has_group("gus")) {
        if (config.has_key("gus", "gus")) {
            this->m_gus_enable_switch->set_active(config.get_boolean("gus", "gus"));
        }

        if (config.has_key("gus", "gusrate")) {
            this->m_gus_sample_rate_cbt->set_active_id(config.get_value("gus", "gusrate"));
        }

        if (config.has_key("gus", "gusbase")) {
            this->m_gus_address_cbt->set_active_id(config.get_value("gus", "gusbase"));
        }

        if (config.has_key("gus", "gusirq")) {
            this->m_gus_irq_cbt->set_active_id(config.get_value("gus", "gusirq"));
        }

        if (config.has_key("gus", "gusdma")) {
            this->m_gus_dma_cbt->set_active_id(config.get_value("gus", "gusdma"));
        }

        if (config.has_key("gus", "ultradir")) {
            this->m_gus_dir_entry->set_text(config.get_value("gus", "ultradir"));
        }
    }

    // speaker group -----------------------------------------------------------
    if (config.has_group("speaker")) {
        if (config.has_key("speaker", "pcspeaker")) {
            this->m_pc_speaker_switch->set_active(config.get_boolean("speaker", "pcspeaker"));
        }

        if (config.has_key("speaker", "pcrate")) {
            this->m_pc_speaker_sample_rate_cbt->set_active_id(config.get_value("speaker", "pcrate"));
        }

        if (config.has_key("speaker", "tandy")) {
            this->m_tandy_enable_cbt->set_active_id(config.get_value("speaker", "tandy"));
        }

        if (config.has_key("speaker", "tandyrate")) {
            this->m_tandy_sample_rate_cbt->set_active_id(config.get_value("speaker", "tandyrate"));
        }

        if (config.has_key("speaker", "disney")) {
            this->m_disney_switch->set_active(config.get_boolean("speaker", "disney"));
        }
    }

    // joystick group ----------------------------------------------------------
    if (config.has_group("joystick")) {
        if (config.has_key("joystick", "joysticktype")) {
            this->m_joystick_type_cbt->set_active_id(config.get_value("joystick", "joysticktype"));
        }

        if (config.has_key("joystick", "timed")) {
            this->m_timed_switch->set_active(config.get_boolean("joystick", "timed"));
        }

        if (config.has_key("joystick", "autofire")) {
            this->m_auto_fire_switch->set_active(config.get_boolean("joystick", "autofire"));
        }

        if (config.has_key("joystick", "swap34")) {
            this->m_swap34_switch->set_active(config.get_boolean("joystick", "swap34"));
        }

        if (config.has_key("joystick", "buttonwrap")) {
            this->m_button_wrap_switch->set_active(config.get_boolean("joystick", "buttonwrap"));
        }
    }

    // serial group ------------------------------------------------------------
    if (config.has_group("serial")) {
        config.set_list_separator(' ');
        if (config.has_key("serial", "serial1")) {
            std::vector<Glib::ustring> values = config.get_string_list("serial", "serial1");

            this->m_serial1_type_cbt->set_active_id(values[0]);

            for (int index = 1; index < values.size(); ++index) {
                this->m_serial1_parameters_entry->set_text(this->m_serial1_parameters_entry->get_text() + " " + values[index]);
            }
        }

        if (config.has_key("serial", "serial2")) {
            std::vector<Glib::ustring> values = config.get_string_list("serial", "serial2");

            this->m_serial2_type_cbt->set_active_id(values[0]);

            for (int index = 1; index < values.size(); ++index) {
                this->m_serial2_parameters_entry->set_text(this->m_serial2_parameters_entry->get_text() + " " + values[index]);
            }
        }

        if (config.has_key("serial", "serial3")) {
            std::vector<Glib::ustring> values = config.get_string_list("serial", "serial3");

            this->m_serial3_type_cbt->set_active_id(values[0]);

            for (int index = 1; index < values.size(); ++index) {
                this->m_serial3_parameters_entry->set_text(this->m_serial3_parameters_entry->get_text() + " " + values[index]);
            }
        }

        if (config.has_key("serial", "serial4")) {
            std::vector<Glib::ustring> values = config.get_string_list("serial", "serial4");

            this->m_serial4_type_cbt->set_active_id(values[0]);

            for (int index = 1; index < values.size(); ++index) {
                this->m_serial4_parameters_entry->set_text(this->m_serial4_parameters_entry->get_text() + " " + values[index]);
            }
        }
    }

    // dos group ---------------------------------------------------------------
    if (config.has_group("dos")) {
        if (config.has_key("dos", "xms")) {
            this->m_xms_switch->set_active(config.get_boolean("dos", "xms"));
        }

        if (config.has_key("dos", "ems")) {
            this->m_ems_switch->set_active(config.get_boolean("dos", "ems"));
        }

        if (config.has_key("dos", "umb")) {
            this->m_umb_switch->set_active(config.get_boolean("dos", "umb"));
        }

        if (config.has_key("dos", "keyboardlayout")) {
            this->m_keyboard_layout_cbt->set_active_id(config.get_value("dos", "keyboardlayout"));
        }
    }

    // ipx group ---------------------------------------------------------------
    if (config.has_group("ipx")) {
        if (config.has_key("ipx", "ipx")) {
            this->m_ipx_switch->set_active(config.get_boolean("ipx", "ipx"));
        }
    }
}

/**
 * Saves the profile's DOSBox config file ofr the main program and for the setup
 * rogram if there's one.
 */
void EditProfileDialog::save_config_file()
{
    auto profiles_path         = this->m_settings->get_string("profiles-path"),
         config_basename       = Glib::ustring::compose("%1.conf", this->m_profile_id),
         setup_config_basename = Glib::ustring::compose("%1_setup.conf", this->m_profile_id);
    Glib::KeyFile default_config, config;
    Glib::ustring autoexec_program = this->create_autoexec(),
                  autoexec_setup, value_string;
    bool value_boolean;
    int value_integer;

    if (!this->m_setup_entry->get_text().empty()) {
        autoexec_setup =  this->create_autoexec(true);
    }

    config.set_comment(Glib::ustring::compose(_(" DOSBox config file for '%1'\n"
                                                " This config file was generated by %2 version %3.%4."),
                                              this->m_title_entry->get_text(),
                                              PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR));

    default_config.load_from_file(this->m_settings->get_string("default-config"));

    // sdl group ---------------------------------------------------------------
    value_boolean = this->m_fullscreen_switch->get_active();

    if (value_boolean != default_config.get_boolean("sdl", "fullscreen")) {
        config.set_boolean("sdl", "fullscreen", value_boolean);
    }

    value_boolean = this->m_double_buffering_switch->get_active();

    if (value_boolean != default_config.get_boolean("sdl", "fulldouble")) {
        config.set_boolean("sdl", "fulldouble", value_boolean);
    }

    value_string = this->m_full_resolution_cbt->get_active_text();

    if (value_string != default_config.get_value("sdl", "fullresolution")) {
        config.set_value("sdl", "fullresolution", value_string);
    }

    value_string = this->m_windowed_resolution_cbt->get_active_text();

    if (value_string != default_config.get_value("sdl", "windowresolution")) {
        config.set_value("sdl", "windowresolution", value_string);
    }

    value_string = this->m_outut_cbt->get_active_id();

    if (value_string != default_config.get_value("sdl", "output")) {
        config.set_value("sdl", "output", value_string);
    }

    value_boolean = this->m_mouse_autolock_switch->get_active();

    if (value_boolean != default_config.get_boolean("sdl", "autolock")) {
        config.set_boolean("sdl", "autolock", value_boolean);
    }

    value_integer = this->m_mouse_sensitivity_spin_button->get_value_as_int();

    if (value_integer != default_config.get_integer("sdl", "sensitivity")) {
        config.set_integer("sdl", "sensitivity", value_integer);
    }

    value_boolean = this->m_wait_on_error_switch->get_active();

    if (value_boolean != default_config.get_boolean("sdl", "waitonerror")) {
        config.set_boolean("sdl", "waitonerror", value_boolean);
    }

    value_string = Glib::ustring::compose("%1,%2",
                                          this->m_priority_active_cbt->get_active_id(),
                                          this->m_priority_inactive_cbt->get_active_id());

    if (value_string != default_config.get_value("sdl", "priority")) {
        config.set_value("sdl", "priority", value_string);
    }

    value_string = this->m_mapper_file_entry->get_text();

    if (value_string != default_config.get_value("sdl", "mapperfile")) {
        config.set_value("sdl", "mapperfile", value_string);
    }

    value_boolean = this->m_keyboard_scancodes_switch->get_active();

    if (value_boolean != default_config.get_boolean("sdl", "usescancodes")) {
        config.set_boolean("sdl", "usescancodes", value_boolean);
    }

    // dosbox group ------------------------------------------------------------
    value_string = this->m_language_file_fcb->get_filename();

    if (value_string != default_config.get_value("dosbox", "language")) {
        config.set_value("dosbox", "language", value_string);
    }

    value_string = this->m_machine_cbt->get_active_id();

    if (value_string != default_config.get_value("dosbox", "machine")) {
        config.set_value("dosbox", "machine", value_string);
    }

    value_string = this->m_captures_fcb->get_filename();

    if (value_string != default_config.get_value("dosbox", "captures")) {
        config.set_value("dosbox", "captures", value_string);
    }

    value_integer = this->m_memory_size_spin_button->get_value_as_int();

    if (value_integer != default_config.get_integer("dosbox", "memsize")) {
        config.set_integer("dosbox", "memsize", value_integer);
    }

    // render group ------------------------------------------------------------
    value_integer = this->m_frame_skip_spin_button->get_value_as_int();

    if (value_integer != default_config.get_integer("render", "frameskip")) {
        config.set_integer("render", "frameskip", value_integer);
    }

    value_boolean = this->m_aspect_correction_switch->get_active();

    if (value_boolean != default_config.get_boolean("render", "aspect")) {
        config.set_boolean("render", "aspect", value_boolean);
    }

    value_string = this->m_scaler_cbt->get_active_id();

    if (this->m_forced_scaler_switch->get_active()) {
        value_string += " forced";
    }

    if (value_string != default_config.get_value("render", "scaler")) {
        config.set_value("render", "scaler", value_string);
    }

    // cpu group ---------------------------------------------------------------
    value_string = this->m_core_cbt->get_active_id();

    if (value_string != default_config.get_value("cpu", "core")) {
        config.set_value("cpu", "core", value_string);
    }

    value_string = this->m_cpu_type_cbt->get_active_id();

    if(value_string != default_config.get_value("cpu", "cputype")) {
        config.set_value("cpu", "cputype", value_string);
    }

    value_string = this->m_cycles_cbt->get_active_id();

    if (value_string == "fixed") {
        value_string += " " + Glib::Ascii::dtostr(this->m_cycles_spin_button->get_value());
    }

    if (value_string != default_config.get_value("cpu", "cycles")) {
        config.set_value("cpu", "cycles", value_string);
    }

    value_integer = this->m_cycle_up_spin_button->get_value_as_int();

    if (value_integer != default_config.get_integer("cpu", "cycleup")) {
        config.set_integer("cpu", "cycleup", value_integer);
    }

    value_integer = this->m_cycle_down_spin_button->get_value_as_int();

    if (value_integer != default_config.get_integer("cpu", "cycledown")) {
        config.set_integer("cpu", "cycledown", value_integer);
    }

    // mixer group -------------------------------------------------------------
    value_boolean = this->m_silent_mode_switch->get_active();

    if (value_boolean != default_config.get_boolean("mixer", "nosound")) {
        config.set_boolean("mixer", "nosound", value_boolean);
    }

    value_string = this->m_general_sample_rate_cbt->get_active_id();

    if (value_string != default_config.get_value("mixer", "rate")) {
        config.set_value("mixer", "rate", value_string);
    }

    value_string = this->m_block_size_cbt->get_active_id();

    if (value_string != default_config.get_value("mixer", "blocksize")) {
        config.set_value("mixer", "blocksize", value_string);
    }

    value_integer = this->m_prebuffer_spin_button->get_value_as_int();

    if (value_integer != default_config.get_integer("mixer", "prebuffer")) {
        config.set_integer("mixer", "prebuffer", value_integer);
    }

    // midi group --------------------------------------------------------------
    value_string = this->m_mpu401_cbt->get_active_id();

    if (value_string != default_config.get_value("midi", "mpu401")) {
        config.set_value("midi", "mpu401", value_string);
    }

    value_string = this->m_midi_device_cbt->get_active_id();

    if (value_string != default_config.get_value("midi", "mididevice")) {
        config.set_value("midi", "mididevice", value_string);
    }

    value_string = this->m_midi_config_entry->get_text();

    if (value_string != default_config.get_value("midi", "midiconfig")) {
        config.set_value("midi", "midiconfig", value_string);
    }

    // sblaster group ----------------------------------------------------------
    value_string = this->m_sb_type_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "sbtype")) {
        config.set_value("midi", "sbtype", value_string);
    }

    value_string = this->m_sb_address_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "sbbase")) {
        config.set_value("midi", "sbbase", value_string);
    }

    value_string = this->m_sb_irq_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "irq")) {
        config.set_value("midi", "irq", value_string);
    }

    value_string = this->m_sb_dma_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "dma")) {
        config.set_value("midi", "dma", value_string);
    }

    value_string = this->m_sb_hdma_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "hdma")) {
        config.set_value("midi", "hdma", value_string);
    }

    value_boolean = this->m_sb_mixer_switch->get_active();

    if (value_boolean != default_config.get_boolean("sblaster", "sbmixer")) {
        config.set_boolean("midi", "sbmixer", value_boolean);
    }


    value_string = this->m_sb_opl_mode_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "oplmode")) {
        config.set_value("midi", "oplmode", value_string);
    }

    value_string = this->m_sb_opl_emulation_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "oplemu")) {
        config.set_value("midi", "oplemu", value_string);
    }

    value_string = this->m_sb_sample_rate_cbt->get_active_id();

    if (value_string != default_config.get_value("sblaster", "oplrate")) {
        config.set_value("midi", "oplrate", value_string);
    }

    // gus group ---------------------------------------------------------------
    value_boolean = this->m_gus_enable_switch->get_active();

    if (value_boolean != default_config.get_boolean("gus", "gus")) {
        config.set_boolean("gus", "gus", value_boolean);
    }

    value_string = this->m_gus_sample_rate_cbt->get_active_id();

    if (value_string != default_config.get_value("gus", "gusrate")) {
        config.set_value("gus", "gusrate", value_string);
    }

    value_string = this->m_gus_address_cbt->get_active_id();

    if (value_string != default_config.get_value("gus", "gusbase")) {
        config.set_value("gus", "gusbase", value_string);
    }

    value_string = this->m_gus_irq_cbt->get_active_id();

    if (value_string != default_config.get_value("gus", "gusirq")) {
        config.set_value("gus", "gusirq", value_string);
    }

    value_string = this->m_gus_dma_cbt->get_active_id();

    if (value_string != default_config.get_value("gus", "gusdma")) {
        config.set_value("gus", "gusdma", value_string);
    }

    value_string = this->m_gus_dir_entry->get_text();

    if (value_string != default_config.get_value("gus", "ultradir")) {
        config.set_value("gus", "ultradir", value_string);
    }

    // speaker group -----------------------------------------------------------
    value_boolean = this->m_pc_speaker_switch->get_active();

    if (value_boolean != default_config.get_boolean("speaker", "pcspeaker")) {
        config.set_boolean("speaker", "pcspeaker", value_boolean);
    }

    value_string = this->m_pc_speaker_sample_rate_cbt->get_active_id();

    if (value_string != default_config.get_value("speaker", "pcrate")) {
        config.set_value("speaker", "pcrate", value_string);
    }

    value_string = this->m_tandy_enable_cbt->get_active_id();

    if (value_string != default_config.get_value("speaker", "tandy")) {
        config.set_value("speaker", "tandy", value_string);
    }

    value_string = this->m_tandy_sample_rate_cbt->get_active_id();

    if (value_string != default_config.get_value("speaker", "tandyrate")) {
        config.set_value("speaker", "tandyrate", value_string);
    }

    value_boolean = this->m_disney_switch->get_active();

    if (value_boolean != default_config.get_boolean("speaker", "disney")) {
        config.set_boolean("speaker", "disney", value_boolean);
    }

    // joystick group ----------------------------------------------------------
    value_string = this->m_joystick_type_cbt->get_active_id();

    if (value_string != default_config.get_value("joystick", "joysticktype")) {
        config.set_value("speaker", "joystick", value_string);
    }

    value_boolean = this->m_timed_switch->get_active();

    if (value_boolean != default_config.get_boolean("joystick", "timed")) {
        config.set_boolean("joystick", "timed", value_boolean);
    }

    value_boolean = this->m_auto_fire_switch->get_active();

    if (value_boolean != default_config.get_boolean("joystick", "autofire")) {
        config.set_boolean("joystick", "autofire", value_boolean);
    }

    value_boolean = this->m_swap34_switch->get_active();

    if (value_boolean != default_config.get_boolean("joystick", "swap34")) {
        config.set_boolean("joystick", "swap34", value_boolean);
    }

    value_boolean = this->m_button_wrap_switch->get_active();

    if (value_boolean != default_config.get_boolean("joystick", "buttonwrap")) {
        config.set_boolean("joystick", "buttonwrap", value_boolean);
    }

    // serial group ------------------------------------------------------------
    value_string = this->m_serial1_type_cbt->get_active_id();

    if (!this->m_serial1_parameters_entry->get_text().empty()) {
        value_string += Glib::ustring::compose(" %1", this->m_serial1_parameters_entry->get_text());
    }

    if (value_string != default_config.get_value("serial", "serial1")) {
        config.set_value("serial", "serial1", value_string);
    }

    value_string = this->m_serial2_type_cbt->get_active_id();

    if (!this->m_serial2_parameters_entry->get_text().empty()) {
        value_string += Glib::ustring::compose(" %1", this->m_serial2_parameters_entry->get_text());
    }

    if (value_string != default_config.get_value("serial", "serial2")) {
        config.set_value("serial", "serial2", value_string);
    }

    value_string = this->m_serial3_type_cbt->get_active_id();

    if (!this->m_serial3_parameters_entry->get_text().empty()) {
        value_string += Glib::ustring::compose(" %1", this->m_serial3_parameters_entry->get_text());
    }

    if (value_string != default_config.get_value("serial", "serial3")) {
        config.set_value("serial", "serial3", value_string);
    }

    value_string = this->m_serial4_type_cbt->get_active_id();

    if (!this->m_serial4_parameters_entry->get_text().empty()) {
        value_string += Glib::ustring::compose(" %1", this->m_serial4_parameters_entry->get_text());
    }

    if (value_string != default_config.get_value("serial", "serial4")) {
        config.set_value("serial", "serial4", value_string);
    }

    // dos group ---------------------------------------------------------------
    value_boolean = this->m_xms_switch->get_active();

    if (value_boolean != default_config.get_boolean("dos", "xms")) {
        config.set_boolean("dos", "xms", value_boolean);
    }

    value_boolean = this->m_ems_switch->get_active();

    if (value_boolean != default_config.get_boolean("dos", "ems")) {
        config.set_boolean("dos", "ems", value_boolean);
    }

    value_boolean = this->m_umb_switch->get_active();

    if (value_boolean != default_config.get_boolean("dos", "umb")) {
        config.set_boolean("dos", "umb", value_boolean);
    }

    value_string = this->m_keyboard_layout_cbt->get_active_id();

    if (value_string != default_config.get_value("dos", "keyboardlayout")) {
        config.set_value("dos", "keyboardlayout", value_string);
    }

    // ipx group ---------------------------------------------------------------
    value_boolean = this->m_ipx_switch->get_active();

    if (value_boolean != default_config.get_boolean("ipx", "ipx")) {
        config.set_boolean("ipx", "ipx", value_boolean);
    }

    Glib::file_set_contents(Glib::build_filename(profiles_path, config_basename), config.to_data() + "\n" + autoexec_program);

    if (!this->m_setup_entry->get_text().empty()) {
        Glib::file_set_contents(Glib::build_filename(profiles_path, setup_config_basename), config.to_data() + "\n" + autoexec_setup);
    }
}

/**
 * Removes comment lines from the given config files and splits it in order to
 * separate the autoexec group from the rest of the config file.
 * @param filename Config file filename.
 * @return std::vector with the parts of the splitted config file. Index 0 for
 * the config file contents witout the autoexec group and following indexes for
 * the autoexec groups contents, if there is an autoexec group.
 */
std::vector<Glib::ustring> EditProfileDialog::autoexec_split(const Glib::ustring &filename)
{
    Glib::ustring contents = Glib::file_get_contents(filename);

    return Glib::Regex::split_simple("^\\s*\\[autoexec\\]\\s*$", contents, Glib::REGEX_MULTILINE, Glib::REGEX_MATCH_NEWLINE_ANY);
}

/**
 * Parses a text line with the given regular expression
 * @param line Line to parse.
 * @param pcre_expresion PCRE for parsing the text line.
 * @param minfo Glib::MachInfo object with the parsing results.
 * @return @c TRUE if the regular expresion matches or @c FALSE otherwise.
 */
bool EditProfileDialog::parse_line(const Glib::ustring &line, const Glib::ustring &pcre_expresion, Glib::MatchInfo &minfo) const
{
    auto comments_regex = Glib::Regex::create("^\\s*#.*$", Glib::REGEX_MULTILINE);
    bool matched = false;

    if (!comments_regex->match(line)) {
        auto regex = Glib::Regex::create(pcre_expresion, Glib::REGEX_CASELESS);
        matched = regex->match(line, 0, minfo);
    }

    return matched;
}

/**
 * Parses the autoexec group of the config file in order to set the related
 * profile control's values.
 * @param autoexec Contents of a config file autoexec group.
 * @param for_setup If is @c TRUE the autoexec will be parsed to retrieve only
 * the setup information (executable and parameters).
 */
void EditProfileDialog::parse_autoexec(const Glib::ustring &autoexec, bool for_setup)
{
    auto lines = Glib::Regex::split_simple("\n", autoexec);
    Glib::MatchInfo minfo;
    Glib::ustring drive_letter, mount_path, path, program, parameters;
    auto exec_entry       = this->m_program_entry,
         parameters_entry = this->m_program_parameters_entry;

    if (for_setup) {
        exec_entry       = this->m_setup_entry;
        parameters_entry = this->m_setup_parameters_entry;
    }

    for (auto line : lines) {
        if (!for_setup && this->parse_line(line, PCRE_KEYB, minfo)) {
             this->m_keyb_args_entry->set_text(minfo.fetch_named("keyb_args"));
        } else if (!for_setup && this->parse_line(line, PCRE_MIXER, minfo)) {
            this->m_mixer_command = line;
        } else if (!for_setup && this->parse_line(line, PCRE_LOADFIX, minfo)) {
            this->m_loadfix_cb->set_active();
            this->m_loadfix_spin_button->set_value(Glib::Ascii::strtod(minfo.fetch_named("amount")));
        } else if (!for_setup && this->parse_line(line, PCRE_MOUNT, minfo)) {
            this->add_mounting_command(line);
        } else if (this->parse_line(line, PCRE_DRIVE, minfo)) {
            drive_letter = minfo.fetch_named("drive");
        } else if (this->parse_line(line, PCRE_PATH, minfo)) {
            path = minfo.fetch_named("path");
        } else if (!for_setup && this->parse_line(line, PCRE_EXIT, minfo)) {
            this->m_exit_afterwards_switch->set_active();
        } else if (this->parse_line(line, PCRE_PROGRAM, minfo)) {
            this->m_program_rb->set_active();
            this->m_loadhigh_cb->set_active(minfo.fetch_named("loadhigh").uppercase() == "LOADHIGH");
            program = minfo.fetch_named("program");
            parameters = minfo.fetch_named("parameters");
        } else if (!for_setup && this->parse_line(line, PCRE_BOOT, minfo)) {
            auto booter_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_booter_tree_view->get_model());

            do {
                auto letter   = minfo.fetch_named("letter"),
                     image_sq = minfo.fetch_named("image_sq"),
                     image_dq = minfo.fetch_named("image_dq"),
                     image    = minfo.fetch_named("image");

                if (!letter.empty()) {
                    this->m_booter_drive_letter_cbt->set_active_id(letter);
                }

                if (!image_sq.empty()) {
                    image = image_sq;
                } else if (!image_dq.empty()) {
                    image = image_dq;
                }

                if (!image.empty()) {
                    auto iter = booter_ls->append();

                    iter->set_value(0, image);
                }
            } while (minfo.next());

            this->m_booter_rb->set_active();
        }
    }

    auto rows = this->m_mounting_overview_tree_view->get_model()->children();
    auto iter = rows.begin();

    while (iter != rows.end() && mount_path.empty()) {
        Glib::ustring command;

        iter->get_value(0, command);
        this->parse_line(command, PCRE_MOUNT, minfo);

        if (minfo.fetch_named("command").uppercase() == "MOUNT") {
            MountCommand m_command(command);

            if (Glib::ustring(1, m_command.get_letter()) == drive_letter) {
                mount_path = m_command.get_host_dir();
            }
        }

        ++iter;
    }

    program = Glib::build_filename(mount_path, path, program);
    exec_entry->set_text(program);
    parameters_entry->set_text(parameters);
}

/**
 * Creates and returns the autoexec group contents for the config file using the
 * related widget's values.
 * @param for_setup If it is @c TRUE the autoexec group will be created for the
 * setup executable and not for the program.
 * @return The autoexec group contents, including the group header.
 */
Glib::ustring EditProfileDialog::create_autoexec(bool for_setup) const
{
    Glib::ustring autoexec;
    auto exec_entry       = this->m_program_entry,
         parameters_entry = this->m_program_parameters_entry;

    if (for_setup) {
        exec_entry       = this->m_setup_entry;
        parameters_entry = this->m_setup_parameters_entry;
    }

    auto mounting_command_str = this->get_mounting_command_for_program(exec_entry->get_text());

    if (this->m_program_rb->get_active()) {
        if (!this->m_mixer_command.empty()) {
            autoexec += Glib::ustring::compose("%1\n", this->m_mixer_command);
        }

        if (!this->m_keyb_args_entry->get_text().empty()) {
            autoexec += Glib::ustring::compose("KEYB.COM %1\n", this->m_keyb_args_entry->get_text());
        }

        if (this->m_loadfix_cb->get_active()) {
            autoexec += Glib::ustring::compose("LOADFIX.COM -%1\n", this->m_loadfix_spin_button->get_value());
        }
    }

    for (auto row : this->m_mounting_overview_tree_view->get_model()->children()) {
        Glib::ustring mounting_command;

        row->get_value(0, mounting_command);
        autoexec += mounting_command + "\n";
    }

    if (!mounting_command_str.empty()) {
        MountCommand m_command(mounting_command_str);
        auto abs_program_path = exec_entry->get_text();
        auto abs_program_dir_path = Glib::path_get_dirname(abs_program_path),
             rel_program_dir_path = abs_program_dir_path.substr(m_command.get_host_dir().size()),
             program_name = Glib::path_get_basename(abs_program_path);

        autoexec += Glib::ustring::compose("%1:\n", m_command.get_letter());
        if (!rel_program_dir_path.empty()) {
            autoexec += Glib::ustring::compose("CD %1\n", rel_program_dir_path);
        }

        if (this->m_loadhigh_cb->get_active()) {
            autoexec += "LOADHIGH ";
        }

        autoexec += program_name + "\n";
    }

    if (this->m_program_rb->get_active()) {
        if (this->m_loadfix_cb->get_active()) {
            autoexec += "LOADFIX.COM -f\n";
        }

        if (this->m_exit_afterwards_switch->get_active()) {
            autoexec += "EXIT\n";
        }
    }

    if (this->m_booter_rb->get_active()) {
        autoexec += Glib::ustring::compose("BOOT.COM -l %1", this->m_booter_drive_letter_cbt->get_active_text());

        for (auto row : this->m_booter_tree_view->get_model()->children()) {
            Glib::ustring image;
            Glib::ustring quote;
            auto has_spaces_regex = Glib::Regex::create("\\s");

            row->get_value(0, image);

            if (has_spaces_regex->match(image)) {
                quote = "\"";
            }

            autoexec += Glib::ustring::compose(" %1%2%1", quote, image);
        }

        autoexec += "\n";
    }

    return autoexec.empty() ? autoexec : "[autoexec]\n" + autoexec;
}

/**
 * Adds the given mounting command to the mounting overview TreeView.
 * @param command String with the mounting DOSBox command.
 * @return @c TRUE if the command was succesfully added or @c FALSE otherwise.
 */
bool EditProfileDialog::add_mounting_command(const Glib::ustring &command)
{
    auto mounting_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_mounting_overview_tree_view->get_model());
    auto rows = mounting_ls->children();
    auto iter = rows.begin();
    bool found = 0;

    while (iter != rows.end() && !found) {
        Glib::ustring row_command;

        iter->get_value(0, row_command);
        found = command == row_command;
        ++iter;
    }

    if (iter == rows.end()) {
        iter = mounting_ls->append();
        iter->set_value(0, command);
    }

    return !found;
}

/**
 * Gets the drive letters alredy used by the mounting commands.
 * @param ignore_selected_row If @c TRUE the drive letter of the selected row
 * on the mounting TreeView will be ignored.
 * @return String with the used drive letters.
 */
Glib::ustring EditProfileDialog::get_used_letters(bool ignore_selected_row) const
{
    Glib::ustring letters;
    char selected_letter = '\0';
    auto model = this->m_mounting_overview_tree_view->get_model();
    auto selection = this->m_mounting_overview_tree_view->get_selection();
    MountCommandBase *mounting_command = nullptr;

    if (ignore_selected_row && selection->count_selected_rows() == 1) {
        auto selected_row = model->get_iter(selection->get_selected_rows()[0]);
        Glib::MatchInfo minfo;
        Glib::ustring command;

        selected_row->get_value(0, command);
        this->parse_line(command, PCRE_MOUNT, minfo);

        if (minfo.fetch_named("command").uppercase() == "MOUNT") {
            mounting_command = new MountCommand(command);
        } else {
            mounting_command = new ImgmountCommand(command);
        }

        selected_letter = mounting_command->get_letter();

        delete mounting_command;
        mounting_command = nullptr;
    }

    for (auto row : model->children()) {
        Glib::ustring command;
        Glib::MatchInfo minfo;

        row->get_value(0, command);
        this->parse_line(command, PCRE_MOUNT, minfo);

        if (minfo.fetch_named("command").uppercase() == "MOUNT") {
            mounting_command = new MountCommand(command);
        } else {
            mounting_command = new ImgmountCommand(command);
        }

        letters += mounting_command->get_letter();
        delete mounting_command;
    }

    if (ignore_selected_row) {
        letters.erase(letters.find(selected_letter), 1);
    }

    return letters;
}

/**
 * Gets the mounting command that matches the given program.
 * @param program_path Pah of the DOSBox executable whose mounting command is
 * requested.
 * @return Mounting command which corresponds to the given program path or empty
 * string if none is found or the program path is invalid.
 */
Glib::ustring EditProfileDialog::get_mounting_command_for_program(const Glib::ustring &program_path) const
{
    auto is_dosbox_executable_regex = Glib::Regex::create("^[^\\s]+\\.(?:exe|com|bat)$", Glib::REGEX_CASELESS);
    Glib::ustring result_command;

    if (!program_path.empty() && Glib::file_test(program_path, Glib::FILE_TEST_IS_REGULAR) && is_dosbox_executable_regex->match(program_path)) {
        auto rows = this->m_mounting_overview_tree_view->get_model()->children();
        auto iter = rows.begin();

        while (iter != rows.end() && result_command.empty()) {
            Glib::ustring command;
            Glib::MatchInfo minfo;


            iter->get_value(0, command);
            this->parse_line(command, PCRE_MOUNT, minfo);

            if (minfo.fetch_named("command").uppercase() == "MOUNT") {
                MountCommand m_command(command);

                if(Glib::str_has_prefix(program_path, m_command.get_host_dir())) {
                    result_command = command;
                }
            }

            ++iter;
        }
    }

    return result_command;
}

/**
 * Checks if the specified DOSBox program is a valid path and it's contained in
 * one of the mounting points.
 * @param program_path String with the full path to the DOSBox program.
 * @return @c TRUE if the program is correct or @c FALSE otherwise.
 */
bool EditProfileDialog::check_program(const Glib::ustring &program_path) const
{
    return !this->get_mounting_command_for_program(program_path).empty();
}

/**
 * Gets the next available profile ID.
 * @return String with the next available profile ID.
 */
Glib::ustring EditProfileDialog::get_next_id() const
{
    auto profiles_path = this->m_settings->get_string("profiles-path");
    xmlpp::DomParser parser(Glib::build_filename(profiles_path, PROFILES_FILENAME));
    auto root = parser.get_document()->get_root_node();
    guint index = 0;
    Glib::ustring xpath("/profiles/profile[@id='0'][1]");

    while (root->find(xpath).size() > 0) {
        ++index;
        xpath = Glib::ustring::compose("/profiles/profile[@id='%1'][1]", index);
    }

    return Glib::ustring::compose("%1", index);
}

/**
 * Sets the sensitivity of the dialog's Accept Button accordingly to the values
 * of the controls.
 */
void EditProfileDialog::validate_controls()
{
    auto accept_widget = this->get_widget_for_response(Gtk::RESPONSE_ACCEPT);

    if (this->m_program_rb->get_active()) {
        accept_widget->set_sensitive(!this->m_program_entry->get_style_context()->has_class("invalid") &&
                                     !this->m_setup_entry->get_style_context()->has_class("invalid") &&
                                     !this->m_title_entry->get_style_context()->has_class("invalid"));
    } else {
        auto model = this->m_booter_tree_view->get_model();

        accept_widget->set_sensitive(!this->m_title_entry->get_style_context()->has_class("invalid") &&
                                     !model->children().size() == 0);
    }
}

/**
 * Constructor.
 * @param cobject Underlying C object for the Base Class constructor.
 * @param builder Gtk::Builder used to retrieve the child widgets.
 */
EditProfileDialog::EditProfileDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) :
    Gtk::Dialog(cobject), m_settings(Gio::Settings::create(APP_ID, APP_PATH))
{
    Glib::ustring css_data = ".invalid {background-image: none; background-color: rgba(255, 0, 0, .5);}";
    auto css_provider = Gtk::CssProvider::create();
    auto lang_filter = Gtk::FileFilter::create(),
         text_filter = Gtk::FileFilter::create(),
         all_filter  = Gtk::FileFilter::create();

    // Getting dialog's widgets ------------------------------------------------
    builder->set_translation_domain(PROJECT_NAME);
    builder->get_widget("MountingOverviewGrid",       this->m_mounting_overview_grid);
    builder->get_widget("ProgramGrid",                this->m_program_grid);
    builder->get_widget("BooterGrid",                 this->m_booter_grid);

    builder->get_widget("ProgramRB",                  this->m_program_rb);
    builder->get_widget("BooterRB",                   this->m_booter_rb);

    builder->get_widget("ExitAfterwardsSwitch",       this->m_exit_afterwards_switch);
    builder->get_widget("FullScreenSwitch",           this->m_fullscreen_switch);
    builder->get_widget("DoubleBufferingSwitch",      this->m_double_buffering_switch);
    builder->get_widget("MouseAutolockSwitch",        this->m_mouse_autolock_switch);
    builder->get_widget("WaitOnErrorSwitch",          this->m_wait_on_error_switch);
    builder->get_widget("KeyboardScancodesSwitch",    this->m_keyboard_scancodes_switch);
    builder->get_widget("AspectCorrectionSwitch",     this->m_aspect_correction_switch);
    builder->get_widget("ForcedScalerSwitch",         this->m_forced_scaler_switch);
    builder->get_widget("SilentModeSwitch",           this->m_silent_mode_switch);
    builder->get_widget("SBMixerSwitch",              this->m_sb_mixer_switch);
    builder->get_widget("GUSEnableSwitch",            this->m_gus_enable_switch);
    builder->get_widget("PCSpeakerSwitch",            this->m_pc_speaker_switch);
    builder->get_widget("DisneySwitch",               this->m_disney_switch);
    builder->get_widget("TimedSwitch",                this->m_timed_switch);
    builder->get_widget("AutoFireSwitch",             this->m_auto_fire_switch);
    builder->get_widget("Swap34Switch",               this->m_swap34_switch);
    builder->get_widget("ButtonWrapSwitch",           this->m_button_wrap_switch);
    builder->get_widget("XMSSwitch",                  this->m_xms_switch);
    builder->get_widget("EMSSwitch",                  this->m_ems_switch);
    builder->get_widget("UMBSwitch",                  this->m_umb_switch);
    builder->get_widget("IPXSwitch",                  this->m_ipx_switch);

    builder->get_widget("LoadhighCB",                 this->m_loadhigh_cb);
    builder->get_widget("LoadfixCB",                  this->m_loadfix_cb);

    builder->get_widget("TitleEntry",                 this->m_title_entry);
    builder->get_widget("DeveloperEntry",             this->m_developer_entry);
    builder->get_widget("PublisherEntry",             this->m_publisher_entry);
    builder->get_widget("GenreEntry",                 this->m_genre_entry);
    builder->get_widget("YearEntry",                  this->m_year_entry);
    builder->get_widget("Serial1ParametersEntry",     this->m_serial1_parameters_entry);
    builder->get_widget("Serial2ParametersEntry",     this->m_serial2_parameters_entry);
    builder->get_widget("Serial3ParametersEntry",     this->m_serial3_parameters_entry);
    builder->get_widget("Serial4ParametersEntry",     this->m_serial4_parameters_entry);
    builder->get_widget("MapperFileEntry",            this->m_mapper_file_entry);
    builder->get_widget("KeybArgsEntry",              this->m_keyb_args_entry);
    builder->get_widget("ProgramEntry",               this->m_program_entry);
    builder->get_widget("ProgramParametersEntry",     this->m_program_parameters_entry);
    builder->get_widget("SetupEntry",                 this->m_setup_entry);
    builder->get_widget("SetupParametersEntry",       this->m_setup_parameters_entry);
    builder->get_widget("MapperFileEntry",            this->m_mapper_file_entry);
    builder->get_widget("MidiConfigEntry",            this->m_midi_config_entry);
    builder->get_widget("GUSDirEntry",                this->m_gus_dir_entry);

    builder->get_widget("FullScreenResolutionCBT",    this->m_full_resolution_cbt);
    builder->get_widget("OutputCBT",                  this->m_outut_cbt);
    builder->get_widget("WindowedResolutionCBT",      this->m_windowed_resolution_cbt);
    builder->get_widget("PriorityActiveCBT",          this->m_priority_active_cbt);
    builder->get_widget("PriorityInactiveCBT",        this->m_priority_inactive_cbt);
    builder->get_widget("CyclesCBT",                  this->m_cycles_cbt);
    builder->get_widget("Serial1TypeCBT",             this->m_serial1_type_cbt);
    builder->get_widget("Serial2TypeCBT",             this->m_serial2_type_cbt);
    builder->get_widget("Serial3TypeCBT",             this->m_serial3_type_cbt);
    builder->get_widget("Serial4TypeCBT",             this->m_serial4_type_cbt);
    builder->get_widget("BooterDriveLetterCBT",       this->m_booter_drive_letter_cbt);
    builder->get_widget("MachineCBT",                 this->m_machine_cbt);
    builder->get_widget("ScalerCBT",                  this->m_scaler_cbt);
    builder->get_widget("CoreCBT",                    this->m_core_cbt);
    builder->get_widget("CPUTypeCBT",                 this->m_cpu_type_cbt);
    builder->get_widget("GeneralSampleRateCBT",       this->m_general_sample_rate_cbt);
    builder->get_widget("BlockSizeCBT",               this->m_block_size_cbt);
    builder->get_widget("MPU401CBT",                  this->m_mpu401_cbt);
    builder->get_widget("MidiDeviceCBT",              this->m_midi_device_cbt);
    builder->get_widget("SBTypeCBT",                  this->m_sb_type_cbt);
    builder->get_widget("SBAddressCBT",               this->m_sb_address_cbt);
    builder->get_widget("SBIRQCBT",                   this->m_sb_irq_cbt);
    builder->get_widget("SBDMACBT",                   this->m_sb_dma_cbt);
    builder->get_widget("SBHDMACBT",                  this->m_sb_hdma_cbt);
    builder->get_widget("SBOPLModeCBT",               this->m_sb_opl_mode_cbt);
    builder->get_widget("SBOPLEmulationCBT",          this->m_sb_opl_emulation_cbt);
    builder->get_widget("SBSampleRateCBT",            this->m_sb_sample_rate_cbt);
    builder->get_widget("GUSSampleRateCBT",           this->m_gus_sample_rate_cbt);
    builder->get_widget("GUSAddressCBT",              this->m_gus_address_cbt);
    builder->get_widget("GUSIRQCBT",                  this->m_gus_irq_cbt);
    builder->get_widget("GUSDMACBT",                  this->m_gus_dma_cbt);
    builder->get_widget("PCSpeakerSampleRateCBT",     this->m_pc_speaker_sample_rate_cbt);
    builder->get_widget("TandyEnableCBT",             this->m_tandy_enable_cbt);
    builder->get_widget("TandySampleRateCBT",         this->m_tandy_sample_rate_cbt);
    builder->get_widget("JoystickTypeCBT",            this->m_joystick_type_cbt);
    builder->get_widget("KeyboardLayoutCBT",          this->m_keyboard_layout_cbt);

    builder->get_widget("LanguageFileFCB",            this->m_language_file_fcb);
    builder->get_widget("CapturesFCB",                this->m_captures_fcb);

    builder->get_widget("CyclesSpinButton",           this->m_cycles_spin_button);
    builder->get_widget("CycleUpSpinButton",          this->m_cycle_up_spin_button);
    builder->get_widget("CycleDownSpinButton",        this->m_cycle_down_spin_button);
    builder->get_widget("LoadfixSpinButton",          this->m_loadfix_spin_button);
    builder->get_widget("MouseSensitivitySpinButton", this->m_mouse_sensitivity_spin_button);
    builder->get_widget("MemorySizeSpinButton",       this->m_memory_size_spin_button);
    builder->get_widget("FrameSkipSpinButton",        this->m_frame_skip_spin_button);
    builder->get_widget("PreBufferSpinButton",        this->m_prebuffer_spin_button);

    builder->get_widget("NotesTextView",              this->m_notes_tv);

    builder->get_widget("MountingOverviewTreeView",   this->m_mounting_overview_tree_view);
    builder->get_widget("BooterTreeView",             this->m_booter_tree_view);

    builder->get_widget("AddMountToolButton",         this->m_add_mount_tb);
    builder->get_widget("EditMountToolButton",        this->m_edit_mount_tb);
    builder->get_widget("RemoveMountToolButton",      this->m_remove_mount_tb);
    builder->get_widget("AddBootToolButton",          this->m_add_boot_tb);
    builder->get_widget("RemoveBootToolButton",       this->m_remove_boot_tb);

    builder->get_widget("ConsultButton",              this->m_consult_button);
    builder->get_widget("MixerButton",                this->m_mixer_button);

    // Setting invalid class. --------------------------------------------------
    css_provider->load_from_data(css_data);
    this->m_program_entry->get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    this->m_setup_entry->get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    this->m_title_entry->get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Setting drive letters for booter section. -------------------------------
    for (char letter = 'A'; letter < 'Z'; ++letter) {
        auto letter_str = Glib::ustring(1, letter);
        this->m_booter_drive_letter_cbt->append(letter_str, letter_str);
    }

    this->m_booter_drive_letter_cbt->set_active(0);

    // Setting the language files FileChooserButton ----------------------------
    lang_filter->set_name(_("DOSBox Language files."));
    lang_filter->add_pattern("*.[Ll][Aa][Nn][Gg]");

    text_filter->set_name(_("Text files"));
    text_filter->add_mime_type("text/plain");
    text_filter->add_pattern("*.[Tt][Xx][Tt]");

    all_filter->set_name(_("All files"));
    all_filter->add_pattern("*");

    this->m_language_file_fcb->add_filter(lang_filter);
    this->m_language_file_fcb->add_filter(text_filter);
    this->m_language_file_fcb->add_filter(all_filter);

    this->m_language_file_fcb->set_current_folder(Glib::get_home_dir());

    // Getting a valid profile ID. ---------------------------------------------
    this->m_profile_id = this->get_next_id();

    // Signals -----------------------------------------------------------------
    this->m_add_mount_tb->signal_clicked().connect(sigc::mem_fun(*this, &EditProfileDialog::on_add_mount_tb_clicked));
    this->m_edit_mount_tb->signal_clicked().connect(sigc::mem_fun(*this, &EditProfileDialog::on_edit_mount_tb_clicked));
    this->m_remove_mount_tb->signal_clicked().connect(sigc::mem_fun(*this, &EditProfileDialog::on_remove_mount_tb_clicked));
    this->m_add_boot_tb->signal_clicked().connect(sigc::mem_fun(*this, &EditProfileDialog::on_add_boot_tb_clicked));
    this->m_remove_boot_tb->signal_clicked().connect(sigc::mem_fun(*this, &EditProfileDialog::on_remove_boot_tb_clicked));
    this->m_program_rb->signal_toggled().connect(sigc::mem_fun(*this, &EditProfileDialog::on_program_rb_toggled));
    this->m_loadfix_cb->signal_toggled().connect(sigc::mem_fun(*this, &EditProfileDialog::on_loadfix_cb_toggled));
    this->m_mixer_button->signal_clicked().connect(sigc::mem_fun(*this, &EditProfileDialog::on_mixer_button_clicked));
    this->m_consult_button->signal_clicked().connect(sigc::mem_fun(*this, &EditProfileDialog::on_consult_button_clicked));
    this->m_cycles_cbt->signal_changed().connect(sigc::mem_fun(*this, &EditProfileDialog::on_cycles_cbt_changed));

    for (auto object : builder->get_objects()) {
        auto entry = Glib::RefPtr<Gtk::Entry>::cast_dynamic(object);
        Glib::RefPtr<Gtk::TreeSelection> selection = Glib::RefPtr<Gtk::TreeSelection>::cast_dynamic(object);

        if (entry) {
            entry->signal_changed().connect(sigc::bind<Gtk::Entry*>(sigc::mem_fun(*this, &EditProfileDialog::on_entry_changed), entry.operator ->()));
            entry->signal_icon_release().connect(sigc::bind<Gtk::Entry*>(sigc::mem_fun(*this, &EditProfileDialog::on_entry_icon_release), entry.operator ->()));
        } else if (selection) {
            selection->signal_changed().connect(sigc::bind<Gtk::TreeView*>(sigc::mem_fun(*this, &EditProfileDialog::on_selection_changed), selection->get_tree_view()));
        }
    }

    // Loading default config file ---------------------------------------------
    this->load_config_file(this->m_settings->get_string("default-config"));
}

/**
 * Loads the given game profile.
 * @param id ID of the profile to be lodaded.
 */
void EditProfileDialog::load_profile(const Glib::ustring &id)
{
    auto profiles_path   = this->m_settings->get_string("profiles-path");
    auto config_filename = Glib::build_filename(profiles_path, Glib::ustring::compose("%1.conf", id)),
         setup_filename  = Glib::build_filename(profiles_path, Glib::ustring::compose("%1_setup.conf", id));
    xmlpp::DomParser parser(Glib::build_filename(profiles_path, PROFILES_FILENAME));
    auto nodes = parser.get_document()->get_root_node()->get_children();
    auto iter = nodes.begin();

    parser.set_substitute_entities();

    while (iter != nodes.end() && static_cast<xmlpp::Element*>(*iter)->get_attribute_value("id") != id) {
        ++iter;
    }

    if (iter == nodes.end()) {
        throw std::invalid_argument(Glib::ustring::compose(_("Invalid profile's ID: Unable to find profile with ID '%1'."), id));
    }

    this->m_profile_id = id;

    for (auto node : (*iter)->get_children()) {
        auto element   = static_cast<xmlpp::Element*>(node);
        auto text_node = element->get_child_text();
        auto name = element->get_name();
        Glib::ustring content;

        if (text_node != nullptr) {
            content = text_node->get_content();
        }

        if (name == "title") {
            this->m_title_entry->set_text(content);
        } else if (name == "developer") {
            this->m_developer_entry->set_text(content);
        } else if (name == "publisher") {
            this->m_publisher_entry->set_text(content);
        } else if (name == "genre") {
            this->m_genre_entry->set_text(content);
        } else if (name == "year") {
            this->m_year_entry->set_text(content);
        } else if (name == "notes") {
            this->m_notes_tv->get_buffer()->set_text(content);
        }
    }

    this->load_config_file(this->m_settings->get_string("default-config"));

    if (Glib::file_test(config_filename, Glib::FILE_TEST_IS_REGULAR)) {
        this->load_config_file(config_filename);
        if (Glib::file_test(setup_filename, Glib::FILE_TEST_IS_REGULAR)) {
            auto setup_autoexec = this->autoexec_split(setup_filename)[1];

            this->parse_autoexec(setup_autoexec, true);
        }
    }

    this->validate_controls();
}

/**
 * Saves the profile to disk.
 */
void EditProfileDialog::save_profile()
{
    auto profiles_path = this->m_settings->get_string("profiles-path");
    xmlpp::DomParser parser(Glib::build_filename(profiles_path, PROFILES_FILENAME));
    auto root = parser.get_document()->get_root_node();
    xmlpp::Element *profile_element   = nullptr;
    auto xpath = Glib::ustring::compose("/profiles/profile[@id='%1'][1]", this->m_profile_id);
    auto result = root->find(xpath);

    if (result.size() > 0) {
        profile_element = static_cast<xmlpp::Element*>(result[0]);

        for (auto node : profile_element->get_children()) {
            auto element      = static_cast<xmlpp::Element*>(node);
            auto text_node    = element->get_child_text();
            auto element_name = element->get_name();

            if (text_node == nullptr) {
                text_node = element->add_child_text(Glib::ustring());
            }

            if (element_name == "title") {
                text_node->set_content(this->m_title_entry->get_text());
            } else if (element_name == "developer") {
                text_node->set_content(this->m_developer_entry->get_text());
            } else if (element_name == "publisher") {
                text_node->set_content(this->m_publisher_entry->get_text());
            } else if (element_name == "genre") {
                text_node->set_content(this->m_genre_entry->get_text());
            } else if (element_name == "year") {
                text_node->set_content(this->m_year_entry->get_text());
            } else if (element_name == "notes") {
                text_node->set_content(this->m_notes_tv->get_buffer()->get_text());
            }
        }
    }

    if (profile_element == nullptr) {
        profile_element = root->add_child("profile");
        profile_element->set_attribute("id", this->m_profile_id);
        profile_element->add_child("title")->set_child_text(this->m_title_entry->get_text());
        profile_element->add_child("developer")->set_child_text(this->m_developer_entry->get_text());
        profile_element->add_child("publisher")->set_child_text(this->m_publisher_entry->get_text());
        profile_element->add_child("genre")->set_child_text(this->m_genre_entry->get_text());
        profile_element->add_child("year")->set_child_text(this->m_year_entry->get_text());
        profile_element->add_child("notes")->set_child_text(this->m_notes_tv->get_buffer()->get_text());
    }

    parser.get_document()->write_to_file_formatted(Glib::build_filename(profiles_path, PROFILES_FILENAME), "UTF-8");
    this->save_config_file();
}

} // DOSBoxGTK
