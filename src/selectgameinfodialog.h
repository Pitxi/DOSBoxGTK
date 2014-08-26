/**
 * @file
 * SelectGameInfoDialog class declaration.
 * @author Javier Campón Pichardo
 * @date 2014/07/28
 * @copyright 2014 Javier Campón Pichardo
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

#ifndef SELECTGAMEINFODIALOG_H
#define SELECTGAMEINFODIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/treeview.h>

/**
 * DOSBocGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * SelectGameInfoDialog class.
 */
class SelectGameInfoDialog final : public Gtk::Dialog
{
private:
    Gtk::TreeView *m_games_tv    = nullptr;
    Gtk::Button *m_accept_button = nullptr;
    Glib::ustring m_base_url;

    void on_response(int response_id);
    void on_games_tv_selection_changed();

public:
    SelectGameInfoDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder);

    void search_game_info(const Glib::ustring &title);
    Glib::ustring get_selected_href() const;
};

} // DOSBoxGTK

#endif // SELECTGAMEINFODIALOG_H
