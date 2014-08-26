/**
 * @file
 * MountCommandBase class declaration.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef DOSBOXCOMMAND_H
#define DOSBOXCOMMAND_H

#include <glibmm/ustring.h>
#include <vector>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Base class for the MountCommand and ImgmountCommand classes.
 */
class MountCommandBase
{
protected:
    char m_drive_letter; ///< The drive letter for the mounting point.

public:
    virtual ~MountCommandBase() {}

    virtual bool parse(const Glib::ustring &command) = 0;
    virtual Glib::ustring get_command() const = 0;
    virtual const char &get_letter() const;
    virtual void clear() = 0;
};

} // DOSBoxGTK

#endif // DOSBOXCOMMAND_H
