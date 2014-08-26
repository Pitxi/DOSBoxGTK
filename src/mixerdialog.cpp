/**
 * @file
 * Implementación de la clase MixerDialog.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "mixerdialog.h"
#include "config.h"
#include <glibmm/stringutils.h>
#include <glibmm/regex.h>
#include <gtkmm/grid.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Process response and closes dialog window.
 * @param response_id Dialog response value;
 */
void MixerDialog::on_response(int response_id)
{
    Gtk::Dialog::on_response(response_id);
    this->hide();
}

/**
 * Handler for the volume controls adjustments.
 * @param sender_adjustment The adjustment which emmitted the signal.
 * @param secondary_adjustment The adjustment for the other audio channel in the
 * same device.
 * @param balance_locker The Gtk::CheckButton that indicates whether the balance is locked or not.
 */
void MixerDialog::on_volume_changed(const Glib::RefPtr<Gtk::Adjustment> sender_adjustment, const Glib::RefPtr<Gtk::Adjustment> secondary_adjustment, Gtk::CheckButton *balance_locker)
{
    if (balance_locker->get_active()) {
        secondary_adjustment->set_value(sender_adjustment->get_value());
    }
}

/**
 * Constructor.
 * @param cobject Underlying C object for the Base Class constructor.
 * @param builder Gtk::Builder used to retrieve the child widgets.
 */
MixerDialog::MixerDialog(Gtk::Dialog::BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) :
    Gtk::Dialog(cobject)
{
    builder->set_translation_domain(PACKAGE);

    this->m_master_left = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("MasterLeftAdjustment"));
    this->m_master_right = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("MasterRightAdjustment"));
    this->m_speaker_left = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("SpeakerLeftAdjustment"));
    this->m_speaker_right = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("SpeakerRightAdjustment"));
    this->m_sb_left = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("SBLeftAdjustment"));
    this->m_sb_right = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("SBRightAdjustment"));
    this->m_gus_left = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("GUSLeftAdjustment"));
    this->m_gus_right = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("GUSRightAdjustment"));
    this->m_tandy_left = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("TandyLeftAdjustment"));
    this->m_tandy_right = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("TandyRightAdjustment"));
    this->m_disney_left = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("DisneyLeftAdjustment"));
    this->m_disney_right = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("DisneyRightAdjustment"));
    this->m_cd_left = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("CDLeftAdjustment"));
    this->m_cd_right = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("CDRightAdjustment"));

    builder->get_widget("MasterLockCB", this->m_master_lock_cb);
    builder->get_widget("SpeakerLockCB", this->m_speaker_lock_cb);
    builder->get_widget("SBLockCB", this->m_sb_lock_cb);
    builder->get_widget("GUSLockCB", this->m_gus_lock_cb);
    builder->get_widget("TandyLockCB", this->m_tandy_lock_cb);
    builder->get_widget("DisneyLockCB", this->m_disney_lock_cb);
    builder->get_widget("CDLockCB", this->m_cd_lock_cb);

    // Master
    this->m_master_left->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_master_left, this->m_master_right, this->m_master_lock_cb));
    this->m_master_right->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_master_right, this->m_master_left, this->m_master_lock_cb));

    // Speaker
    this->m_speaker_left->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_speaker_left, this->m_speaker_right, this->m_speaker_lock_cb));
    this->m_speaker_right->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_speaker_right, this->m_speaker_left, this->m_speaker_lock_cb));

    // Sound Blaster
    this->m_sb_left->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_sb_left, this->m_sb_right, this->m_sb_lock_cb));
    this->m_sb_right->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_sb_right, this->m_sb_left, this->m_sb_lock_cb));

    // Gravis UtraSound
    this->m_gus_left->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_gus_left, this->m_gus_right, this->m_gus_lock_cb));
    this->m_gus_right->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_gus_right, this->m_gus_left, this->m_gus_lock_cb));

    // Tandy FM
    this->m_tandy_left->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_tandy_left, this->m_tandy_right, this->m_tandy_lock_cb));
    this->m_tandy_right->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_tandy_right, this->m_tandy_left, this->m_tandy_lock_cb));

    // Disney Sound Source
    this->m_disney_left->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_disney_left, this->m_disney_right, this->m_disney_lock_cb));
    this->m_disney_right->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_disney_right, this->m_disney_left, this->m_disney_lock_cb));

    // CD Audio
    this->m_cd_left->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_cd_left, this->m_cd_right, this->m_cd_lock_cb));
    this->m_cd_right->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment>, Glib::RefPtr<Gtk::Adjustment>, Gtk::CheckButton*>(sigc::mem_fun(*this, &MixerDialog::on_volume_changed), this->m_cd_right, this->m_cd_left, this->m_cd_lock_cb));
}

/**
 * Gets the DOSBox Mixer command arguments.
 * @return Mixer command.
 */
Glib::ustring MixerDialog::get_mixer_command() const
{
    Glib::ustring arguments;

    double master_left  = this->m_master_left->get_value(),  master_right  = this->m_master_right->get_value(),
           speaker_left = this->m_speaker_left->get_value(), speaker_right = this->m_speaker_right->get_value(),
           sb_left      = this->m_sb_left->get_value(),      sb_right      = this->m_sb_right->get_value(),
           gus_left     = this->m_gus_left->get_value(),     gus_right     = this->m_gus_right->get_value(),
           tandy_left   = this->m_tandy_left->get_value(),   tandy_right   = this->m_tandy_right->get_value(),
           disney_left  = this->m_disney_left->get_value(),  disney_right  = this->m_disney_right->get_value(),
           cd_left      = this->m_cd_left->get_value(),      cd_right      = this->m_cd_right->get_value();


    if (master_left != 100 || master_right != 100) {
        arguments += Glib::ustring::compose(" MASTER %1:%2", master_left, master_right);
    }

    if (speaker_left != 100 || speaker_right != 100) {
        arguments += Glib::ustring::compose(" SPKR %1:%2", speaker_left, speaker_right);
    }

    if (sb_left != 100 || sb_right != 100) {
        arguments += Glib::ustring::compose(" SB %1:%2", sb_left, sb_right);
    }

    if (gus_left != 100 || gus_right != 100) {
        arguments += Glib::ustring::compose(" GUS %1:%2", gus_left, gus_right);
    }

    if (tandy_left != 100 || tandy_right != 100) {
        arguments += Glib::ustring::compose(" FM %1:%2", tandy_left, tandy_right);
    }

    if (disney_left != 100 || disney_right != 100) {
        arguments += Glib::ustring::compose(" DISNEY %1:%2", disney_left, disney_right);
    }

    if (cd_left != 100 || cd_right != 100) {
        arguments += Glib::ustring::compose(" CDAUDIO %1:%2", cd_left, cd_right);
    }

    return arguments.empty() ? arguments : Glib::ustring::compose("MIXER.COM %1", arguments);
}

/**
 * Parse the given mixer command to retrieve the values from it's arguments.
 * @param command DOSBox MIXER.COM command.
 */
void MixerDialog::parse_command(const Glib::ustring &command)
{
    Glib::RefPtr<Glib::Regex> regex = Glib::Regex::create("([a-z]+) ([0-9]+):([0-9]+)", Glib::REGEX_CASELESS);
    Glib::MatchInfo info;

    if (regex->match(command, 0, info)) {
        do {
            Glib::ustring device = info.fetch(1).lowercase();
            double left   = Glib::Ascii::strtod(info.fetch(2)),
                   right  = Glib::Ascii::strtod(info.fetch(3));

            if (device == "master") {
                this->m_master_lock_cb->set_active(left == right);
                this->m_master_left->set_value(left);
                this->m_master_right->set_value(right);
            } else if (device == "spkr") {
                this->m_speaker_lock_cb->set_active(left == right);
                this->m_speaker_left->set_value(left);
                this->m_speaker_right->set_value(right);
            } else if (device == "sb") {
                this->m_sb_lock_cb->set_active(left == right);
                this->m_sb_left->set_value(left);
                this->m_sb_right->set_value(right);
            } else if (device == "gus") {
                this->m_gus_lock_cb->set_active(left == right);
                this->m_gus_left->set_value(left);
                this->m_gus_right->set_value(right);
            } else if (device == "fm") {
                this->m_tandy_lock_cb->set_active(left == right);
                this->m_tandy_left->set_value(left);
                this->m_tandy_right->set_value(right);
            } else if (device == "disney") {
                this->m_disney_lock_cb->set_active(left == right);
                this->m_disney_left->set_value(left);
                this->m_disney_right->set_value(right);
            } else if (device == "cdaudio") {
                this->m_cd_lock_cb->set_active(left == right);
                this->m_cd_left->set_value(left);
                this->m_cd_right->set_value(right);
            }
        } while (info.next());
    }
}

} // DOSBoxGTK
