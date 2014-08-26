# ------------------------------------------------------------------------------
# This module contains the ConfigPackage funtion.
#
# ConfigPackage (PACKAGE VERSION ID_PREFIX INSTALL_PREFIX)
# - PACKAGE: name of the package.
# - VERSION: the package version in the format:
#   MAJOR.MINOR.PATCH.TWEAK
# - ID_PREFIX: prefix of the Application ID. Used also to create the Application
#   Path.
#
# Author: Javier Campón Pichardo
# Date: 26/07/2014
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

function(ConfigPackage PACKAGE VERSION ID_PREFIX)
    set(APP_ID "${ID_PREFIX}.${PACKAGE}")
    string(REPLACE "." "/" APP_PATH "${APP_ID}")
    set(APP_PATH "/${APP_PATH}/")

    string(REGEX MATCHALL "[0-9]+" VERSION_PARTS ${VERSION})
    list(GET VERSION_PARTS 0 PROJECT_VERSION_MAJOR)
    list(GET VERSION_PARTS 1 PROJECT_VERSION_MINOR)
    list(GET VERSION_PARTS 2 PROJECT_VERSION_PATCH)
    list(GET VERSION_PARTS 3 PROJECT_VERSION_TWEAK)

    set(PROJECT_TEMPLATE_DIR "${PROJECT_SOURCE_DIR}/templates")
    set(LOCALE_DIR "$ENV{TEXTDOMAINDIR}")

    # Use DEBUG build type by default.
    set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    if(NOT BUILD_TYPE STREQUAL "RELEASE" OR LOCALE_DIR STREQUAL "")
        set(BUILD_TYPE "DEBUG")
        set(LOCALE_DIR "./locale")
    endif()

    if (EXISTS "${PROJECT_TEMPLATE_DIR}/config_header.in")
        configure_file("${PROJECT_TEMPLATE_DIR}/config_header.in" "${PROJECT_SOURCE_DIR}/src/config.h" @ONLY)
    endif()

    if (EXISTS "${PROJECT_TEMPLATE_DIR}/gschema.xml.in")
        configure_file("${PROJECT_TEMPLATE_DIR}/gschema.xml.in" "${PROJECT_BINARY_DIR}/schemas/${APP_ID}.gschema.xml" @ONLY)
        # Target for compiling the gschemas in the binary dir.
        add_custom_target(gschema ALL
                          COMMAND glib-compile-schemas "${PROJECT_BINARY_DIR}/schemas"
                          COMMENT "Compiling GLib settings schema file..." VERBATIM)
    endif()

    if(EXISTS "${PROJECT_TEMPLATE_DIR}/gresource.xml.in")
        configure_file("${PROJECT_TEMPLATE_DIR}/gresource.xml.in" "${PROJECT_BINARY_DIR}/${APP_ID}.gresource.xml" @ONLY)
        # Embedding resources into the code.
        execute_process(COMMAND glib-compile-resources --generate-source "--sourcedir=${PROJECT_SOURCE_DIR}" "${APP_ID}.gresource.xml" "--target=${PROJECT_SOURCE_DIR}/src/resources.c"
                        COMMAND glib-compile-resources --generate-header "--sourcedir=${PROJECT_SOURCE_DIR}" "${APP_ID}.gresource.xml" "--target=${PROJECT_SOURCE_DIR}/src/resources.h"
                        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                        RESULT_VARIABLE   Result
                        ERROR_VARIABLE    ErrorOutput)
        if(NOT Result EQUAL 0)
            message(FATAL_ERROR "${Red}Error Code ${Result}\nError Message: ${ErrorOutput}${ColourReset}")
        endif()
    endif()

    set(PACKAGE "${PACKAGE}" PARENT_SCOPE)
    set(LOCALE_DIR "${LOCALE_DIR}" PARENT_SCOPE)
    set(PROJECT_TEMPLATE_DIR "${PROJECT_TEMPLATE_DIR}" PARENT_SCOPE)
    set(CMAKE_BUILD_TYPE "${BUILD_TYPE}" PARENT_SCOPE)
    set(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}" PARENT_SCOPE)
    set(PROJECT_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}" PARENT_SCOPE)
    set(PROJECT_VERSION_MINOR "${PROJECT_VERSION_MINOR}" PARENT_SCOPE)
    set(PROJECT_VERSION_PATCH "${PROJECT_VERSION_PATCH}" PARENT_SCOPE)
    set(PROJECT_VERSION_TWEAK "${PROJECT_VERSION_TWEAK}" PARENT_SCOPE)
endfunction()
