# ------------------------------------------------------------------------------
# CMake cross-platform uninstall script.
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

if (NOT EXISTS "/media/javier/Desarrollo/Proyectos/C y C++/DOSBoxGTK/build/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: \"/media/javier/Desarrollo/Proyectos/C y C++/DOSBoxGTK/build/install_manifest.txt\"")
endif()

file(READ "/media/javier/Desarrollo/Proyectos/C y C++/DOSBoxGTK/build/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
list(REVERSE files)
foreach (file ${files})
    message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
    if (EXISTS "$ENV{DESTDIR}${file}")
        execute_process(COMMAND /usr/bin/cmake -E remove "$ENV{DESTDIR}${file}"
                        OUTPUT_VARIABLE rm_out
                        RESULT_VARIABLE rm_retval)
        if(NOT ${rm_retval} EQUAL 0)
            message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
        endif()
    else ()
        message(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
    endif ()
endforeach()
