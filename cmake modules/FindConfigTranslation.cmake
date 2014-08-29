# ------------------------------------------------------------------------------
# This module defines the ConfigTranslation function, which creates the
# necessary targets to help creating and processing the translation files.
#
# ConfigTranslation (COPYRIGHT_HOLDER PACKAGE_BUG_REPORT <filenames...>)
#   Creates the 'pot' target to generate the PO template file and a target for
#   each of the alredy existing PO files in the traslations directory to create
#   the messages object files containing the translated strings. Each of the PO
#   targets are bind to the ALL target.
#   - COPYRIGHT_HOLDER: Name of the copyright holder.
#   - PACKAGE_BUG_REPORT: E-mail address to send the bug reports concerning
#                         translation.
#   - filenames: The filenames from which to extract the strings that can be
#                translated.
#
# Author: Javier Campón Pichardo
# Date: 28/08/2014
# Copyright 2014 Javier Campón Pichardo
#
# Distributeed under the terms of the GNU General Public License version 3 or
# later.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software. If not, see <http://www.gnu.org/licenses/>.
# ------------------------------------------------------------------------------

function(ConfigTranslation COPYRIGHT_HOLDER PACKAGE_BUG_REPORT)
    find_program(XGETTEXT_EXECUTABLE xgettext)
    find_program(MSGFMT_EXECUTABLE msgfmt)

    set(TRANSLATIONS_DIR "${PROJECT_SOURCE_DIR}/translations")
    
    if(NOT XGETTEXT_EXECUTABLE STREQUAL "XGETTEXT_EXECUTABLE-NOTFOUND")
        set(XGETTEXT_FOUND YES)
        add_custom_target(pot
                          mkdir -p "${TRANSLATIONS_DIR}"
                          COMMAND "${XGETTEXT_EXECUTABLE}"
                          "--default-domain=${PACKAGE}" "--output-dir=${TRANSLATIONS_DIR}" "--output=${PACKAGE}.pot"
                          --from-code=UTF-8 --language=C++ --keyword=_ --indent --sort-by-file
                          "--package-name=${PACKAGE}"
                          "--package-version=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}"
                          "--copyright-holder=${COPYRIGHT_HOLDER}" "--msgid-bugs-address=${PACKAGE_BUG_REPORT}"
                          ${ARGN}
                          WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                          COMMENT "Creating POT file at '${TRANSLATIONS_DIR}'..." VERBATIM)

    message(STATUS "${Green}Use '${Blue}make pot${Green}' to create the PO Template file.${ColourReset}")
    endif()

    if(NOT MSGFMT_EXECUTABLE STREQUAL "MSGFMT_EXECUTABLE-NOTFOUND")
        file(GLOB PO_FILES "${TRANSLATIONS_DIR}/*.po")

        foreach(po_file IN LISTS PO_FILES)
            get_filename_component(language "${po_file}" NAME_WE)

            add_custom_target("po-${language}" ALL
                              mkdir -p "${PROJECT_BINARY_DIR}/locale/${language}/LC_MESSAGES"
                              COMMAND "${MSGFMT_EXECUTABLE}"
                              "--output=${PROJECT_BINARY_DIR}/locale/${language}/LC_MESSAGES/${PACKAGE}.mo" "${po_file}"
                              WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                              COMMENT "Processing PO file for locale '${language}'..." VERBATIM)
        endforeach()
    endif()
endfunction()
