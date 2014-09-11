/**
 * @file
 * ResourceManager class implementation.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h>

/**
 * Namespace used for miscelaneous tools and utilities.
 */
namespace Tools
{

/**
 * A manager for retriving the application's embedded resources.
 */
class ResourceManager final
{
private:
    Glib::ustring m_resources_base_path; ///< Base path for the application's resources.

public:
    ResourceManager(const Glib::ustring &resources_base_path);
    Glib::RefPtr<Gdk::Pixbuf> get_image(const Glib::ustring &resource_path) const;
    Glib::ustring get_text(const Glib::ustring &resource_path) const;
    virtual ~ResourceManager() {}
};

} // DOSBoxGTK

#endif // RESOURCEMANAGER_H
