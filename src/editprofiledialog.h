/**
 * @file
 * EditProfileDialog class definition.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef EDITPROFILEDIALOG_H
#define EDITPROFILEDIALOG_H

// Pearlc Compatible Regular Expresions for parsing autoexec group in config file.
#define PCRE_MIXER   "^MIXER" ///< PCRE for MIXER DOSBox Command.
#define PCRE_KEYB    "^KEYB(?:\\.COM){0,1}\\s+(?'keyb_args'.+)" ///< PCRE for KEYB DOSBox Command.
#define PCRE_LOADFIX "^LOADFIX(?:.COM){0,1}\\s+-(?'amount'[0-9]+)$" ///< PCRE for LOADFIX DOSBox Command.
#define PCRE_MOUNT   "^(?'command'IMGMOUNT|MOUNT)" ///< PCRE for MOUNT and IMGMount DOSBox Commands.
#define PCRE_BOOT    "^(?'command'BOOT(?:\\.COM){0,1})|-l\\s+(?'letter'[A-Y])|(?:\\s+'(?'image_sq'[^']+)'|\"(?'image_dq'[^\"]+)\"|(?'image'[^\\s]+))" ///< PCRE for BOOT DOSBox Command.
#define PCRE_DRIVE   "^(?'drive'[A-Y]):" /// < PCRE for the drive letter.
#define PCRE_PATH    "^CD (?'path'.+)" ///< PCRE for CD DOSBox Command.
#define PCRE_PROGRAM "^(?!(?:LOADFIX(?:\\.COM){0,1}|MOUNT(?:\\.COM){0,1}|IMGMOUNT(?:\\.COM){0,1}|MIXER(?:\\.COM){0,1}|KEYB(?:\\.COM){0,1}|BOOT(?:\\.COM){0,1}|EXIT|[A-Z]:|CD(?:\\s+|$)))(?:(?'loadhigh'LOADHIGH)\\s+)?(?'program'[^\\s]+)(?:\\s+(?'parameters'.+))?$" ///< PCER for DOS executables.
#define PCRE_EXIT    "^EXIT" ///< PCRE for EXIT DOSBox Command.

#include "mountcommand.h"
#include <glibmm/keyfile.h>
#include <glibmm/regex.h>
#include <giomm/settings.h>
#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/switch.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>
#include <gtkmm/toolbutton.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Dialog for game profile edition.
 */
class EditProfileDialog final : public Gtk::Dialog
{
private:
    Gtk::Grid *m_mounting_overview_grid              = nullptr,
              *m_program_grid                        = nullptr,
              *m_booter_grid                         = nullptr;
    Gtk::RadioButton *m_program_rb                   = nullptr,
                     *m_booter_rb                    = nullptr;
    Gtk::Switch *m_exit_afterwards_switch            = nullptr,
                *m_fullscreen_switch                 = nullptr,
                *m_double_buffering_switch           = nullptr,
                *m_mouse_autolock_switch             = nullptr,
                *m_wait_on_error_switch              = nullptr,
                *m_keyboard_scancodes_switch         = nullptr,
                *m_aspect_correction_switch          = nullptr,
                *m_forced_scaler_switch              = nullptr,
                *m_silent_mode_switch                = nullptr,
                *m_sb_mixer_switch                   = nullptr,
                *m_gus_enable_switch                 = nullptr,
                *m_pc_speaker_switch                 = nullptr,
                *m_disney_switch                     = nullptr,
                *m_timed_switch                      = nullptr,
                *m_auto_fire_switch                  = nullptr,
                *m_swap34_switch                     = nullptr,
                *m_button_wrap_switch                = nullptr,
                *m_xms_switch                        = nullptr,
                *m_ems_switch                        = nullptr,
                *m_umb_switch                        = nullptr,
                *m_ipx_switch                        = nullptr;
    Gtk::CheckButton *m_loadfix_cb                   = nullptr,
                     *m_loadhigh_cb                  = nullptr;
    Gtk::Entry *m_title_entry                        = nullptr,
               *m_developer_entry                    = nullptr,
               *m_publisher_entry                    = nullptr,
               *m_genre_entry                        = nullptr,
               *m_year_entry                         = nullptr,
               *m_serial1_parameters_entry           = nullptr,
               *m_serial2_parameters_entry           = nullptr,
               *m_serial3_parameters_entry           = nullptr,
               *m_serial4_parameters_entry           = nullptr,
               *m_mapper_file_entry                  = nullptr,
               *m_keyb_args_entry                    = nullptr,
               *m_program_entry                      = nullptr,
               *m_program_parameters_entry           = nullptr,
               *m_setup_entry                        = nullptr,
               *m_setup_parameters_entry             = nullptr,
               *m_midi_config_entry                  = nullptr,
               *m_gus_dir_entry                      = nullptr;
    Gtk::ComboBoxText *m_full_resolution_cbt         = nullptr,
                      *m_outut_cbt                   = nullptr,
                      *m_windowed_resolution_cbt     = nullptr,
                      *m_priority_active_cbt         = nullptr,
                      *m_priority_inactive_cbt       = nullptr,
                      *m_cycles_cbt                  = nullptr,
                      *m_serial1_type_cbt            = nullptr,
                      *m_serial2_type_cbt            = nullptr,
                      *m_serial3_type_cbt            = nullptr,
                      *m_serial4_type_cbt            = nullptr,
                      *m_booter_drive_letter_cbt     = nullptr,
                      *m_machine_cbt                 = nullptr,
                      *m_scaler_cbt                  = nullptr,
                      *m_core_cbt                    = nullptr,
                      *m_cpu_type_cbt                = nullptr,
                      *m_general_sample_rate_cbt     = nullptr,
                      *m_block_size_cbt              = nullptr,
                      *m_mpu401_cbt                  = nullptr,
                      *m_midi_device_cbt             = nullptr,
                      *m_sb_type_cbt                 = nullptr,
                      *m_sb_address_cbt              = nullptr,
                      *m_sb_irq_cbt                  = nullptr,
                      *m_sb_dma_cbt                  = nullptr,
                      *m_sb_hdma_cbt                 = nullptr,
                      *m_sb_opl_mode_cbt             = nullptr,
                      *m_sb_opl_emulation_cbt        = nullptr,
                      *m_sb_sample_rate_cbt          = nullptr,
                      *m_gus_sample_rate_cbt         = nullptr,
                      *m_gus_address_cbt             = nullptr,
                      *m_gus_irq_cbt                 = nullptr,
                      *m_gus_dma_cbt                 = nullptr,
                      *m_pc_speaker_sample_rate_cbt  = nullptr,
                      *m_tandy_enable_cbt            = nullptr,
                      *m_tandy_sample_rate_cbt       = nullptr,
                      *m_joystick_type_cbt           = nullptr,
                      *m_keyboard_layout_cbt         = nullptr;
    Gtk::FileChooserButton *m_language_file_fcb      = nullptr,
                           *m_captures_fcb           = nullptr;
    Gtk::SpinButton *m_cycles_spin_button            = nullptr,
                    *m_cycle_up_spin_button          = nullptr,
                    *m_cycle_down_spin_button        = nullptr,
                    *m_loadfix_spin_button           = nullptr,
                    *m_mouse_sensitivity_spin_button = nullptr,
                    *m_memory_size_spin_button       = nullptr,
                    *m_frame_skip_spin_button        = nullptr,
                    *m_prebuffer_spin_button         = nullptr;
    Gtk::TextView *m_notes_tv                        = nullptr;
    Gtk::TreeView *m_mounting_overview_tree_view     = nullptr,
                  *m_booter_tree_view                = nullptr;
    Gtk::ToolButton *m_add_mount_tb                  = nullptr,
                    *m_edit_mount_tb                 = nullptr,
                    *m_remove_mount_tb               = nullptr,
                    *m_add_boot_tb                   = nullptr,
                    *m_remove_boot_tb                = nullptr;
    Gtk::Button *m_consult_button                    = nullptr,
                *m_mixer_button                      = nullptr;

    Glib::RefPtr<Gio::Settings> m_settings;
    Glib::ustring m_mixer_command,
                  m_profile_id;

    void on_response(int response_id);
    void on_entry_changed(Gtk::Entry *sender);
    void on_entry_icon_release(Gtk::EntryIconPosition icon_pos, const GdkEventButton *event, Gtk::Entry *sender);
    void on_program_rb_toggled();
    void on_loadfix_cb_toggled();
    void on_add_mount_tb_clicked();
    void on_edit_mount_tb_clicked();
    void on_remove_mount_tb_clicked();
    void on_add_boot_tb_clicked();
    void on_remove_boot_tb_clicked();
    void on_mixer_button_clicked();
    void on_consult_button_clicked();
    void on_cycles_cbt_changed();
    void on_selection_changed(Gtk::TreeView *tv);

    void load_config_file(const Glib::ustring &filename);
    void save_config_file();
    std::vector<Glib::ustring> autoexec_split(const Glib::ustring &filename);
    bool parse_line(const Glib::ustring &line, const Glib::ustring &pcre_expresion, Glib::MatchInfo &minfo) const;
    void parse_autoexec(const Glib::ustring &autoexec, bool for_setup = false);
    Glib::ustring create_autoexec(bool for_setup = false) const;
    bool add_mounting_command(const Glib::ustring &command);
    Glib::ustring get_used_letters(bool ignore_selected_row = false) const;
    Glib::ustring get_mounting_command_for_program(const Glib::ustring &program_path) const;
    bool check_program(const Glib::ustring &program_path) const;
    Glib::ustring get_next_id() const;
    void validate_controls();

public:
    EditProfileDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder);

    void load_profile(const Glib::ustring &id);
    void save_profile();
};

} // DOSBoxGTK

#endif // EDITPROFILEDIALOG_H
