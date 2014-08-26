/**
 * @file
 * ImgmountCommand class declaration.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef IMGMOUNTCOMMAND_H
#define IMGMOUNTCOMMAND_H

#include "mountcommandbase.h"

#define PCRE_IMGMOUNT_COMMAND "^(?'command'IMGMOUNT)(?:\\.COM){0,1}\\s+|(?:-t\\s+(?'image_type'[^\\s]+))|(?:-fs\\s+(?'file_system'[^\\s]+))|\"(?'dq_image'[^\"]+)\"|'(?'sq_image'[^']+)'|(?'image'[^\\s]+)" ///< PCRE for the IMGMOUNT DOSBox command.

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Clase ImgmountCommand
 */
class ImgmountCommand final : public MountCommandBase
{
private:
    std::vector<Glib::ustring> m_images; ///< Images mounted by the command.
    Glib::ustring m_image_type;          ///< Type of the mounted images.

public:
    ImgmountCommand(const Glib::ustring &imgmount_command);
    ImgmountCommand(char letter, const std::vector<Glib::ustring> &images, const Glib::ustring &type = "cdrom");

    bool parse(const Glib::ustring &imgmount_command) override;
    const Glib::ustring &get_image_type() const;
    Glib::ustring get_command() const override;
    const std::vector<Glib::ustring> &get_images() const;
    void clear() override;
};

} // DOSBoxGTK

#endif // IMGMOUNTCOMMAND_H
