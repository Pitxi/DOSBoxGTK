/**
 * @file
 * SelectGameInfoDialog class implementation.
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

#include "selectgameinfodialog.h"
#include "config.h"
#include "htmltools.hpp"
#include <glibmm/regex.h>
#include <gtkmm/liststore.h>
#include <glibmm/convert.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

/**
 * DOSBocGTK namespace.
 */
namespace DOSBoxGTK
{

/**
 * Process response and closes dialog window.
 * @param response_id Dialog response value;
 */
void SelectGameInfoDialog::on_response(int response_id)
{
    Gtk::Dialog::on_response(response_id);
    this->hide();
}

/**
 * Sets the sensitivity of the Accept button when the games TreeView selection
 * changes.
 */
void SelectGameInfoDialog::on_games_tv_selection_changed()
{
    auto selection = this->m_games_tv->get_selection();

    this->m_accept_button->set_sensitive(selection->count_selected_rows() > 0);
}

/**
 * Constructor.
 * @param cobject Underlying C object for the Base Class constructor.
 * @param builder Gtk::Builder used to retrieve the child widgets.
 */
SelectGameInfoDialog::SelectGameInfoDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) :
    Gtk::Dialog(cobject), m_base_url("http://www.mobygames.com")
{
    builder->set_translation_domain(PROJECT_NAME);

    builder->get_widget("GamesTV", this->m_games_tv);
    builder->get_widget("AcceptButton", this->m_accept_button);

    // Signals
    this->m_games_tv->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &SelectGameInfoDialog::on_games_tv_selection_changed));
}

/**
 * Searchs the info about the provided game title on MobyGames website.
 * @param title Title of the game.
 */
void SelectGameInfoDialog::search_game_info(const Glib::ustring &title)
{
    curlpp::initialize();
    curlpp::Easy request;
    std::stringstream os;
    Glib::MatchInfo m_info;
    auto post_fields = Glib::ustring::compose("game=%1&p=2&search=go", curlpp::escape(title));
    auto games_ls = Glib::RefPtr<Gtk::ListStore>::cast_static(this->m_games_tv->get_model());
    auto regex = Glib::Regex::create("Game:\\s*<a\\s+href=\"(?'href'.+?)\">\\s*(?'title'.+?)\\s*<\\/a>\\s*.+?\\s*DOS\\s*\\(<em>\\s*(?'year'.+?)\\s*<\\/em>\\)");

    request.setOpt<curlpp::options::Url>(this->m_base_url + "/search/quick");
    request.setOpt<curlpp::options::PostFields>(post_fields);
    request.setOpt<curlpp::options::WriteStream>(&os);

    request.perform();
    curlpp::terminate();
    regex->match(os.str(), 0, m_info);

    do {
        auto title = Tools::html_entities_decode(m_info.fetch_named("title"));
        auto href  = m_info.fetch_named("href"),
             year  = m_info.fetch_named("year");
        auto iter = games_ls->append();

        iter->set_value(0, title);
        iter->set_value(1, year);
        iter->set_value(2, href);
    } while (m_info.next());
}

/**
 * Gets the selected HREF of the game selected in the TreeView.
 * @return String with the HREF of the selected game.
 */
Glib::ustring SelectGameInfoDialog::get_selected_href() const
{
    auto selection = this->m_games_tv->get_selection();
    Glib::ustring href;

    if (selection->count_selected_rows() > 0) {
        auto iter = selection->get_selected();

        iter->get_value(2, href);
    }

    return this->m_base_url + href;
}

} // DOSBoxGTK
