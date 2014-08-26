/**
 * @file
 * MountCommand class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "mountcommand.h"
#include <glibmm/stringutils.h>
#include <glibmm/regex.h>
#include <iostream>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Consructor.
 * @param mount_command DOSBox MOUNT command represented by this class.
 */
MountCommand::MountCommand(const Glib::ustring &mount_command)
{
    this->parse(mount_command);
}

/**
 * Constructor.
 * @param letter Drive letter for the mounting point.
 * @param host_dir Host directory to be mounted.
 * @param type Type of mounting.
 * @param label Volume label.
 * @param cd_access CD access layer.
 * @param usecd SDL CD number.
 * @param freesize Mounting point free size.
 */
MountCommand::MountCommand(char letter, const Glib::ustring &host_dir, const Glib::ustring &type, const Glib::ustring &label,
                           const Glib::ustring &cd_access, int usecd, int freesize)
{
    this->m_drive_letter = letter;
    this->m_host_dir     = host_dir;
    this->m_type         = type;
    this->m_label        = label;
    this->m_cd_access    = cd_access;
    this->m_usecd        = usecd;
    this->m_freesize     = freesize;
}

/**
 * Parses a DOSBox MOUNT command to get it's arguments.
 * @param mount_command Command to be parsed.
 * @return @c TRUE if the given command is a valid MOUNT command or @c FALSE
 * otherwise.
 */
bool MountCommand::parse(const Glib::ustring &mount_command)
{
    auto mount_regex = Glib::Regex::create(PCRE_MOUNT_COMMAND, Glib::REGEX_CASELESS);
    Glib::MatchInfo minfo;
    bool valid_command = mount_regex->match(mount_command, 0, minfo);

    valid_command = valid_command && minfo.fetch_named("command").uppercase() == "MOUNT";
    this->clear();

    if (valid_command) {
        do {
            auto drive_letter  = minfo.fetch_named("drive_letter").uppercase(),
                 host_dir      = minfo.fetch_named("host_dir"),
                 dq_host_dir   = minfo.fetch_named("dq_host_dir"),
                 sq_host_dir   = minfo.fetch_named("sq_host_dir"),
                 option        = minfo.fetch_named("option"),
                 single_option = minfo.fetch_named("single_option"),
                 value         = minfo.fetch_named("value"),
                 dq_value      = minfo.fetch_named("dq_value"),
                 sq_value      = minfo.fetch_named("sq_value");

            if (!drive_letter.empty()) {
                this->m_drive_letter = drive_letter.c_str()[0];
            }

            if (!dq_host_dir.empty()) {
                host_dir = dq_host_dir;
            } else if (!sq_host_dir.empty()) {
                host_dir = sq_host_dir;
            }

            if (!host_dir.empty()) {
                this->m_host_dir = Glib::strescape(host_dir);
            }

            if (!dq_value.empty()) {
                value = dq_value;
            } else if (!sq_value.empty()) {
                value = sq_value;
            }

            if (!value.empty()) {
                if (option == "-t") {
                    this->m_type = value;
                } else if (option == "-label") {
                    this->m_label = value;
                } if (option == "-usecd") {
                    this->m_usecd = Glib::Ascii::strtod(value);
                } else if (option == "-freesize") {
                    this->m_freesize = Glib::Ascii::strtod(value);
                }
            } else {
                if (single_option == "-ioctl") {
                    this->m_cd_access = "ioctl";
                } else if (single_option == "-ioctl_dx") {
                    this->m_cd_access = "ioctl_dx";
                } else if (single_option == "-ioctl_dio") {
                    this->m_cd_access = "ioctl_dio";
                } else if (single_option == "-ioctl_mci") {
                    this->m_cd_access = "ioctl_mci";
                } else if (single_option == "-noioctl") {
                    this->m_cd_access = "noioctl";
                } else if (single_option == "aspi") {
                    this->m_cd_access = "aspi";
                }
            }
        } while (minfo.next());
    }

    valid_command = !this->m_host_dir.empty() && this->m_drive_letter != '\0';

    if (!valid_command) {
        this->clear();
    }

    return valid_command;
}

/**
 * Gets the MOUNT command.
 * @return MOUNT DOSBox command.
 */
Glib::ustring MountCommand::get_command() const
{
    Glib::ustring command;

    if (!this->m_host_dir.empty() && this->m_drive_letter != '\0') {
        auto has_spaces_regex = Glib::Regex::create("\\s");
        Glib::ustring quote;

        if (has_spaces_regex->match(this->m_host_dir)) {
            quote = "\"";
        }

        command = Glib::ustring::compose("MOUNT.COM %1 %2%3%2", this->m_drive_letter, quote, this->m_host_dir);

        if (!this->m_type.empty()) {
            command += Glib::ustring::compose(" -t %1", this->m_type);
        }

        if (!this->m_label.empty()) {
            command += Glib::ustring::compose(" -label %1", this->m_label);
        }

        if (this->m_freesize > -1) {
            command += Glib::ustring::compose(" -freesize %1", this->m_freesize);
        }

        if (this->m_usecd > -1) {
            command += Glib::ustring::compose(" -usecd %1", this->m_usecd);
        }

        if (!this->m_cd_access.empty()) {
            command += Glib::ustring::compose(" -%1", this->m_cd_access);
        }
    }

    return command;
}

/**
 * Clears the object.
 */
void MountCommand::clear()
{
    this->m_drive_letter = '\0';
    this->m_freesize     = -1;
    this->m_host_dir     = Glib::ustring();
    this->m_cd_access    = Glib::ustring();
    this->m_label        = Glib::ustring();
    this->m_type         = Glib::ustring();
    this->m_usecd        = -1;
}

/**
 * Gets the host dir to be mounted.
 * @return Host dir.
 */
const Glib::ustring &MountCommand::get_host_dir() const
{
    return this->m_host_dir;
}

/**
 * Gets the CD access level of the MOUNT command.
 * @return CD access level.
 */
const Glib::ustring &MountCommand::get_cd_access() const
{
    return this->m_cd_access;
}

/**
 * Gets the volume label of the MOUNT command.
 * @return Volume label.
 */
const Glib::ustring &MountCommand::get_label() const
{
    return this->m_label;
}

/**
 * Gets the mount type of the MOUNT command.
 * @return Mount type.
 */
const Glib::ustring &MountCommand::get_type() const
{
    return this->m_type;
}

/**
 * Gets the drive's available free size.
 * @return Free size.
 */
int MountCommand::get_freesize() const
{
    return this->m_freesize;
}

/**
 * Gets the SDL CDROM drive number.
 * @return CDROM drive number.
 */
int MountCommand::get_usecd() const
{
    return this->m_usecd;
}

} // DOSBoxGTK
