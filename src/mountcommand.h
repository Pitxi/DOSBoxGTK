/**
 * @file
 * MountCommand class declaration.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef MOUNTCOMMAND_H
#define MOUNTCOMMAND_H

#include "mountcommandbase.h"

#define PCRE_MOUNT_COMMAND "^(?'command'MOUNT)(?:\\.COM){0,1}\\s+(?'drive_letter'[A-Y])\\s+(?:\"(?'dq_host_dir'[^\"]+)\"|'(?'sq_host_dir'[^']+)'|(?'host_dir'[^-\\s][^\\s]*))|\\s+(?:(?'option'-t|-label|-usecd|-freesize|-size)\\s+(?:(?:\"(?'dq_value'[^\"]+)\")|'(?'sq_value'[^']+)'|(?'value'[^-\\s][^\\s]*)))|\\s+(?'single_option'-ioctl|-ioctl_dx|-ioctl_dio|-ioctl_mci|-noioctl|-aspi)" ///< PCRE for the MOUNT DOSBox command.

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * This class represents a DOSBox MOUNT or IMGMOUNT command with its arguments.
 */
class MountCommand final : public MountCommandBase
{
private:
    Glib::ustring m_host_dir,  ///< Host's dir to be mounted.
                  m_cd_access, ///< IOCTL option.
                  m_label,     ///< Volume label.
                  m_type;      ///< Type of mounting point (dir, cdrom or floppy)
             int m_freesize,   ///< Drive's free size.
                 m_usecd;      ///< Number of SDL cdrom device to use.

public:
    MountCommand(const Glib::ustring &mount_command);
    MountCommand(char letter, const Glib::ustring &host_dir, const Glib::ustring &type = "dir", const Glib::ustring &label = Glib::ustring(),
                 const Glib::ustring &cd_access = Glib::ustring(), int usecd = -1, int freesize = -1);

    bool parse(const Glib::ustring &mount_command) override;
    Glib::ustring get_command() const override;
    void clear() override;
    const Glib::ustring &get_host_dir() const;
    const Glib::ustring &get_cd_access() const;
    const Glib::ustring &get_label() const;
    const Glib::ustring &get_type() const;
    int get_freesize() const;
    int get_usecd() const;
};

} // DOSBoxGTK

#endif // MOUNTCOMMAND_H
