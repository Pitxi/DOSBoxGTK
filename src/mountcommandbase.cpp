/**
 * @file
 * MountCommandBase class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "mountcommandbase.h"
#include <glibmm/i18n.h>
#include <glibmm/regex.h>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Gets the dirve letter of the mounting point.
 * @return Drive letter.
 */
const char &MountCommandBase::get_letter() const
{
    return this->m_drive_letter;
}

} // DOSBoxGTK
