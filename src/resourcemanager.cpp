/**
 * @file
 * ResourceManager class definition.
 * @author Javier Campón Pichardo
 * @date 2014
 * @copyright GNU Public License Version 3
 */

#include "resourcemanager.hpp"
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <giomm/resource.h>

/**
 * Namespace used for miscelaneous tools and utilities.
 */
namespace Tools
{

/**
 * Constructor.
 */
ResourceManager::ResourceManager(const Glib::ustring &resources_base_path) :
    m_resources_base_path(resources_base_path)
{}

/**
 * Retrieve an image from an embedded resource.
 * @param resource_path Relative path to the resource.
 * @return Reference to a Gdk::Pixbuf object with the requested image.
 */
Glib::RefPtr<Gdk::Pixbuf> ResourceManager::get_image(const Glib::ustring &resource_path) const
{
    auto abs_path = Glib::build_filename(this->m_resources_base_path, resource_path);
    auto glib_is = g_resources_open_stream(abs_path.c_str(), G_RESOURCE_LOOKUP_FLAGS_NONE, nullptr);

    if (!G_IS_INPUT_STREAM(glib_is)) {
        throw Gio::ResourceError(Gio::ResourceError::NOT_FOUND, Glib::ustring::compose("Resource not found: '%1'.", abs_path));
    }

    Glib::RefPtr<Gio::InputStream> stream = Glib::wrap(glib_is);

    return Gdk::Pixbuf::create_from_stream(stream);
}

/**
 * Gets the contents of an embedded text file.
 * @param resource_path Relative path to the resource.
 * @return String with the requested text.
 */
Glib::ustring ResourceManager::get_text(const Glib::ustring &resource_path) const
{
    Glib::ustring path = Glib::build_filename(this->m_resources_base_path, resource_path);
    GBytes *text_bytes = g_resources_lookup_data(path.c_str(), G_RESOURCE_LOOKUP_FLAGS_NONE, nullptr);
    gsize text_bytes_size = g_bytes_get_size(text_bytes);

    return (gchar*)g_bytes_get_data(text_bytes, &text_bytes_size);
}

} // DOSBoxGTK
