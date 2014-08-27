# ------------------------------------------------------------------------------
# DOSBoxGTK
#
# A GTK based DOSBox GUI frontend developed in C++ using GTKmm, LibXML++ and
# cURLpp libraries.
#
# Author: Javier Campón Pichardo
# Date: 2014
# License: Gnu Public License Version 3
# Author: Javier Campón Pichardo
# Date: 25/07/2014
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

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

# ---------------------
# Project configuration
# ---------------------
project("DOSBoxGTK")
# Include custom modules path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake modules")
find_package(ConfigPackage REQUIRED)
ConfigPackage("dosboxgtk" "1.1.1.0" "org.kazires")

find_package(Colours)

# ----------------
# Compiler options
# ----------------
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# -----------------------
# Libraries configuration
# -----------------------
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKMM REQUIRED "gtkmm-3.0 >= 3.8.1")
pkg_check_modules(LIBXMLPP REQUIRED "libxml++-2.6 >= 2.36.0")
pkg_check_modules(LIBCURLPP REQUIRED "curlpp >= 0.7.3")
pkg_check_modules(LIBCPPTOOLS REQUIRED "libcpptools >= 1.0.0.0")

include_directories(${GTKMM_INCLUDE_DIRS} ${LIBXMLPP_INCLUDE_DIRS} ${LIBCURLPP_INCLUDE_DIRS} ${LIBCPPTOOLS_INCLUDE_DIRS})
link_directories   (${GTKMM_LIBRARY_DIRS} ${LIBXMLPP_LIBRARY_DIRS} ${LIBCURLPP_LIBRARY_DIRS} ${LIBCPPTOOLS_LIBRARY_DIRS})
add_definitions    (${GTKMM_CFLAGS_OTHER} ${LIBXMLPP_CFLAGS_OTHER} ${LIBCURLPP_CFLAGS_OTHER} ${LIBCPPTOOLS_CFLAGS_OTHER})

# -----------
# Source code
# -----------
set(SOURCES
    src/resources.c
    src/main.cpp
    src/mainwindow.cpp
    src/preferencesdialog.cpp
    src/editprofiledialog.cpp
    src/mixerdialog.cpp
    src/editmountdialog.cpp
    src/mountcommandbase.cpp
    src/mountcommand.cpp
    src/imgmountcommand.cpp
    src/selectgameinfodialog.cpp)

set(HEADERS
    src/config.h
    src/resources.h
    src/mainwindow.h
    src/preferencesdialog.h
    src/editprofiledialog.h
    src/mixerdialog.h
    src/editmountdialog.h
    src/mountcommandbase.h
    src/mountcommand.h
    src/imgmountcommand.h
    src/selectgameinfodialog.h)

set(GLADE_FILES
    gui/mainwindow.glade
    gui/preferencesdialog.glade
    gui/editprofiledialog.glade
    gui/mixerdialog.glade
    gui/editmountdialog.glade
    gui/selectgameinfodialog.glade)

add_executable(${PACKAGE} ${SOURCES} ${HEADERS})
target_link_libraries(${PACKAGE} ${GTKMM_LIBRARIES} ${LIBXMLPP_LIBRARIES} ${LIBCURLPP_LIBRARIES} ${LIBCPPTOOLS_LIBRARIES})

# ---------------------
# Doxygen documentation
# ---------------------
set(PROJECT_BRIEF "GTK based DOSBox GUI frontend.") # Brief description of the project.

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOCUMENTATION_LANGUAGE English) # Documentation language.
    configure_file (${PROJECT_TEMPLATE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY)

    # Use 'make doc' to generate the documentation.
    add_custom_target(doc
                      ${DOXYGEN_EXECUTABLE} Doxyfile
                      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                      COMMENT "Generating documentation with Doxygen" VERBATIM)
endif ()

# ------------
# Translations
# ------------
# - Creates a custom target named pot to create the PO Template file.
# - Creates custom targets for every existing translation. This targets will be
#   built within the ALL target.
#   The PO files with the translations must be at translations directory on the
#   project source directory.
#   The MO files will be created at locale directory in the project binary
#   directory.
# -----------------------------------------------------------------------------

find_package(XGettext REQUIRED)

set(TRANSLATIONS_DIR "${PROJECT_SOURCE_DIR}/translations")

if(MSGFMT_FOUND)
    file(GLOB PO_FILES "${TRANSLATIONS_DIR}/*.po")

    foreach(po_file IN LISTS PO_FILES)
        get_filename_component(language "${po_file}" NAME_WE)

        add_custom_target("po-${language}" ALL
                          mkdir -p "${PROJECT_BINARY_DIR}/locale/${language}/LC_MESSAGES"
                          COMMAND "${MSGFMT_EXECUTABLE}"
                          "--output=${PROJECT_BINARY_DIR}/locale/${language}/LC_MESSAGES/${PACKAGE}.mo" "${po_file}"
                          WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                          COMMENT "Processint PO file for locale '${language}'..." VERBATIM)
    endforeach()
endif()

if(XGETTEXT_FOUND)
    set(COPYRIGHT_HOLDER "Javier Campón Pichardo")
    set(PACKAGE_BUG_REPORT javiercampon@gmail.com)

    add_custom_target(pot
                      mkdir -p "${TRANSLATIONS_DIR}"
                      COMMAND "${XGETTEXT_EXECUTABLE}"
                      "--default-domain=${PACKAGE}" "--output-dir=${TRANSLATIONS_DIR}" "--output=${PACKAGE}.pot"
                      --from-code=UTF-8 --language=C++ --keyword=_ --indent --sort-by-file
                      "--package-name=${PACKAGE}"
                      "--package-version=${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}"
                      "--copyright-holder=${COPYRIGHT_HOLDER}" "--msgid-bugs-address=${PACKAGE_BUG_REPORT}"
                      ${SOURCES} ${HEADERS}
                      COMMAND "${XGETTEXT_EXECUTABLE}"
                      "--default-domain=${PACKAGE}" "--output-dir=${TRANSLATIONS_DIR}" "--output=${PACKAGE}.pot"
                      --from-code=UTF-8 --language=Glade --keyword=_ --indent --sort-by-file --join-existing
                      "--package-name=${PACKAGE}"
                      "--package-version=${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}"
                      "--copyright-holder=${COPYRIGHT_HOLDER}" "--msgid-bugs-address=${PACKAGE_BUG_REPORT}"
                      ${GLADE_FILES}
                      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      COMMENT "Creating POT file..." VERBATIM)

    message(STATUS "${Green}Use '${Blue}make pot${Green}' to create the PO Template file.${ColourReset}")
    message(STATUS "${Green}The absolute path of the POT file is '${Blue}${TRANSLATIONS_DIR}/${PACKAGE}.pot${Green}'.${ColourReset}")
endif()

# --------------
# Install target
# --------------

# Install and uninstall targets will be available only in RELEASE build type.
if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    set(CMAKE_INSTALL_PREFIX "/usr")
    set(GSCHEMA_DIR "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas")

    configure_file("${PROJECT_TEMPLATE_DIR}/desktop.in" "${PROJECT_BINARY_DIR}/${PACKAGE}.desktop" @ONLY)

    install(TARGETS ${PACKAGE}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/locale/" DESTINATION "${LOCALE_DIR}"
            FILES_MATCHING PATTERN "*.mo")
    install(DIRECTORY "${PROJECT_BINARY_DIR}/schemas/" DESTINATION "${GSCHEMA_DIR}"
            FILES_MATCHING PATTERN "*.xml")
    install(FILES "${PROJECT_BINARY_DIR}/${PACKAGE}.desktop" DESTINATION "share/applications") # Desktop file for system menus/dash.
    install(FILES "${PROJECT_SOURCE_DIR}/icons/${PACKAGE}.svg" DESTINATION "share/icons/hicolor/scalable/apps")
    install(CODE "execute_process(COMMAND glib-compile-schemas \"${GSCHEMA_DIR}\")") # Compiling gschemas.
    install(CODE "execute_process(COMMAND gtk-update-icon-cache \"${CMAKE_INSTALL_PREFIX}/share/icons/hicolor\")") # Recreating the icons cache after instalation.

    message(STATUS "${Green}Use '${Blue}make install${Green}' to install the application.${ColourReset}")

    # ----------------
    # Uninstall target
    # ----------------
    configure_file("${PROJECT_TEMPLATE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" @ONLY)
    add_custom_target(uninstall
                      COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                      COMMAND glib-compile-schemas "${GSCHEMA_DIR}"
                      COMMAND gtk-update-icon-cache "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor"
                      COMMENT "Deleting installed files andconfigurations...")

    message(STATUS "${Green}Use '${Blue}make uninstall${Green}' to uninstall the application.${ColourReset}")

    # ------------
    # CPack config
    #-------------
    set(CPACK_GENERATOR DEB)
    set(CPACK_INSTALL_PREFIX /usr)
    set(CPACK_SET_DESTDIR TRUE)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_NAME           "${PACKAGE}")
    set(CPACK_DEBIAN_PACKAGE_VERSION        "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE   "amd64")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER     "javiercamponp@gmail.com")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION    "${PROJECT_BRIEF}")
    set(CPACK_DEBIAN_PACKAGE_SECTION        "Games")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY       "optional")
    set(CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS "dosbox (>=0.74)")
    include(CPack)
endif()
