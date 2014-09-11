/**
 * @file
 * Declaration of helper functions for working with HTML.
 * @author Javier Campón Pichardo
 * @date 2014/07/30
 * @copyright 2014 Javier Campón Pichardo.
 *
 * Distributeed under the terms of the GNU General Public License version 3 or
 * later.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTMLTOOLS_HPP
#define HTMLTOOLS_HPP

#include <glibmm/ustring.h>

/**
 * Namespace used for miscelaneous tools and utilities.
 */
namespace Tools
{

Glib::ustring html_entities_decode(const Glib::ustring &str);
Glib::ustring html_entities_encode(const Glib::ustring &str);

} // Tools

#endif // HTMLTOOLS_HPP
