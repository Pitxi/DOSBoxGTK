/**
 * @file
 * ImgmountCommand class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "imgmountcommand.h"
#include <glibmm/regex.h>
#include <glibmm/stringutils.h>
#include <iostream>

/**
 * DOSBoxGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Constructor.
 * @param imgmount_command DOSBox IMGMount command represented by this class.
 */
ImgmountCommand::ImgmountCommand(const Glib::ustring &imgmount_command)
{
    this->m_drive_letter = '\0';
    this->parse(imgmount_command);
}

/**
 * Constructor.
 * @param letter Drive letter for the mounting point.
 * @param images Vector with the image files to be mounted.
 * @param type Type of mounting (floppy or cdrom).
 * @param file_system
 */
ImgmountCommand::ImgmountCommand(char letter, const std::vector<Glib::ustring> &images, const Glib::ustring &type)
{
    this->m_drive_letter = letter;
    this->m_images.assign(images.begin(), images.end());
    this->m_image_type = type;
}

/**
 * Parses a DOSBox IMGMOUNT command to get it's arguments.
 * @param imgmount_command Command to be parsed.
 * @return @c TRUE if the given command is a valid IMGMOUNT command or @c FALSE
 * otherwise.
 */
bool ImgmountCommand::parse(const Glib::ustring &imgmount_command)
{
    auto imgmount_regex = Glib::Regex::create(PCRE_IMGMOUNT_COMMAND, Glib::REGEX_CASELESS);
    Glib::MatchInfo minfo;
    bool valid_command = imgmount_regex->match(imgmount_command, 0, minfo);

    valid_command = valid_command && minfo.fetch_named("command").uppercase() == "IMGMOUNT";
    this->clear();

    if (valid_command) {
        do {
            auto file_system = minfo.fetch_named("file_system"),
                 image_type  = minfo.fetch_named("image_type"),
                 dq_image    = minfo.fetch_named("dq_image"),
                 sq_image    = minfo.fetch_named("sq_image"),
                 image       = minfo.fetch_named("image");

            if (!image_type.empty()) {
                this->m_image_type = image_type;
            }

            if (!dq_image.empty()) {
                image = dq_image;
            } else if (!sq_image.empty()) {
                image = sq_image;
            }

            if (!image.empty()) {
                this->m_images.push_back(Glib::strescape(image));
            }
        } while (minfo.next());
    }

    valid_command = this->m_images.size() > 0;

    if (!valid_command) {
        this->clear();
    }

    return valid_command;
}

/**
 * Gets the image type of the images.
 * @return Image type.
 */
const Glib::ustring &ImgmountCommand::get_image_type() const
{
    return this->m_image_type;
}

/**
 * Gets the IMGMOUNT command.
 * @return IMGMOUNT DOSBox command.
 */
Glib::ustring ImgmountCommand::get_command() const
{
    Glib::ustring command;

    if (this->m_images.size() > 0 && this->m_drive_letter != '\0') {
        auto has_spaces_regex  = Glib::Regex::create("\\s");

        command = "IMGMOUNT.COM " + Glib::ustring(1, this->m_drive_letter).uppercase();
        std::cout << command << std::endl;

        if (!this->m_image_type.empty()) {
            command += Glib::ustring::compose(" -t %1", this->m_image_type);
        }

        for (auto image : this->m_images) {
            Glib::ustring quote;
            if (has_spaces_regex->match(image)) {
                quote = "\"";
            }

            command += Glib::ustring::compose(" %1%2%1", quote, image);
        }
    }

    return command;
}

/**
 * Gets the disk images to be mounted.
 * @return Vector with the disk images.
 */
const std::vector<Glib::ustring> &ImgmountCommand::get_images() const
{
    return this->m_images;
}

/**
 * Clears the object.
 */
void ImgmountCommand::clear()
{
    this->m_images.clear();
    this->m_drive_letter = '\0';
    this->m_image_type   = Glib::ustring();
}

} // DOSBoxGTK
