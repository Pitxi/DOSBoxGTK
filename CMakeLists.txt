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
ConfigPackage("dosboxgtk" "1.1.2.0" "org.kazires")

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
pkg_check_modules(LIBCPPTOOLS REQUIRED "cpptools >= 1.0.0.0")

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
find_package(ConfigTranslation REQUIRED)
ConfigTranslation("Javier Campón Pichardo" "javiercamponp@gmail.com" _)


# --------------
# Install target
# --------------

# Install and uninstall targets will be available only in RELEASE build type.
if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    set(CMAKE_INSTALL_PREFIX "/usr")
    set(GSCHEMA_DIR "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas")

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
endif()
