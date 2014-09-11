/**
 * @file
 * Implementation of helper functions for working with HTML.
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

#include "htmltools.hpp"
#include <glibmm/stringutils.h>
#include <glibmm/convert.h>
#include <glibmm/regex.h>
#include <map>
#include <iostream>

/**
 * Namespace used for miscelaneous tools and utilities.
 */
namespace Tools
{

/**
 * Constant map with the HTML entities names and their corresponding HTML
 * entity number.
 */
const std::map<Glib::ustring, Glib::ustring> entities {
    {"&nbsp;",   "&#160;"},
    {"&iexcl;",  "&#161;"},
    {"&pound;",  "&#163;"},
    {"&yen;",    "&#165;"},
    {"&brvbar;", "&#166;"},
    {"&sect;",   "&#167;"},
    {"&uml;",    "&#168;"},
    {"&copy;",   "&#169;"},
    {"&laquo;",  "&#171;"},
    {"&not;",    "&#172;"},
    {"&shy;",    "&#173;"},
    {"&reg;",    "&#174;"},
    {"&macr;",   "&#175;"},
    {"&deg;",    "&#176;"},
    {"&plusmn;", "&#177;"},
    {"&sup2;",   "&#178;"},
    {"&sup3;",   "&#179;"},
    {"&acute;",  "&#180;"},
    {"&micro;",  "&#181;"},
    {"&para;",   "&#182;"},
    {"&middot;", "&#183;"},
    {"&cedil;",  "&#184;"},
    {"&sup1;",   "&#185;"},
    {"&ordm;",   "&#186;"},
    {"&raquo;",  "&#187;"},
    {"&frac14;", "&#188;"},
    {"&frac12;", "&#189;"},
    {"&frac34;", "&#190;"},
    {"&iquest;", "&#191;"},
    {"&Agrave;", "&#192;"},
    {"&Aacute;", "&#193;"},
    {"&Acirc;",  "&#194;"},
    {"&Atilde;", "&#195;"},
    {"&Auml;",   "&#196;"},
    {"&Aring;",  "&#197;"},
    {"&Ccedil;", "&#199;"},
    {"&Egrave;", "&#200;"},
    {"&Eacute;", "&#201;"},
    {"&Ecirc;",  "&#202;"},
    {"&Euml;",   "&#203;"},
    {"&Igrave;", "&#204;"},
    {"&Iacute;", "&#205;"},
    {"&Icirc;",  "&#206;"},
    {"&Iuml;",   "&#207;"},
    {"&ETH;",    "&#208;"},
    {"&Ntilde;", "&#209;"},
    {"&Ograve;", "&#210;"},
    {"&Oacute;", "&#211;"},
    {"&Ocirc;",  "&#212;"},
    {"&Otilde;", "&#213;"},
    {"&Ouml;",   "&#214;"},
    {"&times;",  "&#215;"},
    {"&Oslash;", "&#216;"},
    {"&Ugrave;", "&#217;"},
    {"&Uacute;", "&#218;"},
    {"&Ucirc;",  "&#219;"},
    {"&Uuml;",   "&#220;"},
    {"&Yacute;", "&#221;"},
    {"&THORN;",  "&#222;"},
    {"&szlig;",  "&#223;"},
    {"&agrave;", "&#224;"},
    {"&aacute;", "&#225;"},
    {"&acirc;",  "&#226;"},
    {"&atilde;", "&#227;"},
    {"&auml;",   "&#228;"},
    {"&aring;",  "&#229;"},
    {"&aelig;",  "&#230;"},
    {"&ccedil;", "&#231;"},
    {"&egrave;", "&#232;"},
    {"&eacute;", "&#233;"},
    {"&ecirc;",  "&#234;"},
    {"&euml;",   "&#235;"},
    {"&igrave;", "&#236;"},
    {"&iacute;", "&#237;"},
    {"&icirc;",  "&#238;"},
    {"&iuml;",   "&#239;"},
    {"&eth;",    "&#240;"},
    {"&ntilde;", "&#241;"},
    {"&ograve;", "&#242;"},
    {"&oacute;", "&#243;"},
    {"&ocirc;",  "&#244;"},
    {"&otilde;", "&#245;"},
    {"&ouml;",   "&#246;"},
    {"&divide;", "&#247;"},
    {"&oslash;", "&#248;"},
    {"&ugrave;", "&#249;"},
    {"&uacute;", "&#250;"},
    {"&ucirc;",  "&#251;"},
    {"&uuml;",   "&#252;"},
    {"&yacute;", "&#253;"},
    {"&thorn;",  "&#254;"},
    {"&yuml;",   "&#255;"}
};

/**
 * Substitutes the HTML entities in the given string.
 * @param str The string with HTML entities to be substituted.
 * @return the given string with the HTML entities substituted.
 */
Glib::ustring html_entities_decode(const Glib::ustring &str)
{
    Glib::ustring result = str;
    Glib::MatchInfo entity_minfo;
    auto regex_entity = Glib::Regex::create("(?'entity'&.+?;)");

    if(regex_entity->match(str, 0, entity_minfo)) {
        do {
            Glib::MatchInfo code_minfo;
            auto regex_code = Glib::Regex::create("&#(?'entity_code'.+);");
            auto entity = entity_minfo.fetch_named("entity"),
                 number_entity = entity;
            auto iter = entities.find(entity);

            if (iter != entities.end()) {
                number_entity = iter->second;
            }

            if (regex_code->match(number_entity, 0, code_minfo)) {
                auto code = code_minfo.fetch_named("entity_code").lowercase();

                if (*code.begin() == 'x') { // Hexadecimal notation.
                    code = "0" + code;
                }

                auto c = static_cast<gunichar>(Glib::Ascii::strtod(code));
                auto regex_subst = Glib::Regex::create(entity);
                result = regex_subst->replace(result, 0, Glib::ustring(1, c), static_cast<Glib::RegexMatchFlags>(0));
            }
        } while (entity_minfo.next());
    }

    return result;
}

/**
 * Substitutes special chars with HTML entities.
 * @param str Input string with special chars.
 * @return The given string with the special chars substituted.
 */
Glib::ustring html_entities_encode(const Glib::ustring &str)
{
    Glib::ustring result;
    auto found = false;

    for (auto c : str) {
        for (auto pair : entities) {
            auto code = static_cast<gunichar>(Glib::Ascii::strtod(pair.second.substr(2, pair.second.length() - 3)));

            found = code == c;
            if (found) {
                result += pair.first;
                break;
            }
        }

        if (!found) {
            result += c;
        }
    }

    return result;
}

} // Tools
