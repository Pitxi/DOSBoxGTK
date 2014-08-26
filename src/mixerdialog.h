/**
 * @file
 * MixerDialog class declaration.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef MIXERDIALOG_H
#define MIXERDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/checkbutton.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Dialog used to set the sound devices volumes.
 */
class MixerDialog : public Gtk::Dialog
{
private:
    Glib::RefPtr<Gtk::Adjustment> m_master_left, m_master_right,
                                  m_speaker_left, m_speaker_right,
                                  m_sb_left, m_sb_right,
                                  m_gus_left, m_gus_right,
                                  m_tandy_left, m_tandy_right,
                                  m_disney_left, m_disney_right,
                                  m_cd_left, m_cd_right;

    Gtk::CheckButton *m_master_lock_cb  = nullptr,
                     *m_speaker_lock_cb = nullptr,
                     *m_sb_lock_cb      = nullptr,
                     *m_gus_lock_cb     = nullptr,
                     *m_tandy_lock_cb   = nullptr,
                     *m_disney_lock_cb  = nullptr,
                     *m_cd_lock_cb      = nullptr;

protected:
    virtual void on_response(int response_id) override;
    void on_volume_changed(const Glib::RefPtr<Gtk::Adjustment> sender_adjustment, const Glib::RefPtr<Gtk::Adjustment> secondary_adjustment, Gtk::CheckButton *balance_locker);

public:
    MixerDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder);
    virtual ~MixerDialog() {}

    Glib::ustring get_mixer_command() const;
    void parse_command(const Glib::ustring &command);
};

} // DOSBoxGTK

#endif // MIXERDIALOG_H
