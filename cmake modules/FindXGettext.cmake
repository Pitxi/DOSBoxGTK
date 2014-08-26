# CMake Module for XGettext tool
#
# This module looks for the xgettext program of the GNU gettext tools and
# defines the following values:
#  XGETTEXT_EXECUTABLE: the full path to the xgettext program.
#  XGETTEXT_FOUND: True if xgettext has been found.
#
# Author: Javier Campón Pichardo
# Copyleft 2014 Javier Campón Pichardo
#
# Distributed under the GNU GPL version 3 license
#
# This software is distributed WITHOUT ANY WARRANTY; See the license for more
# information.

find_program(XGETTEXT_EXECUTABLE xgettext)
find_program(MSGFMT_EXECUTABLE msgfmt)

if(NOT XGETTEXT_EXECUTABLE STREQUAL "XGETTEXT_EXECUTABLE-NOTFOUND")
    set(XGETTEXT_FOUND YES)
endif()

if(NOT MSGFMT_EXECUTABLE STREQUAL "MSGFMT_EXECUTABLE-NOTFOUND")
    set(MSGFMT_FOUND YES)
endif()
