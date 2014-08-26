DOSBoxGTK
=========

DOSBoxGTK is a DOSBox frontend for Gnu/Linux systems and the Gnome desktop developed in C++ using the GTKmm, Glibmm and XMLpp libraries.

You are free to modify, distribute, execute and compile this software and it's source code under the terms of the Gnu General Public License version 3, just don't forget to mention the source ;-)

Any feedback and help will be appreciated.

this project uses CMake and has some custom targets to facilitate things.
To create a Release version set CMAKE_BUILD_TYPE CMake variable to RELEASE.
To create a Debug version just don't set the CMAKE_BUILD_TYPE variable or set it to DEBUG.

Custom targets:
---------------
- doc: use make doc to generate the Doxygen documentation.
- pot: use make pot to generate the PO Template file.
- install: use make install to install the application (only available on Release build type)
- uninstall: use make uninstall to remove the previously installed application (only available on Release build type. Will need the install_manifest.txt file created on installation.)

Any available po file in the translations dir will be automatically processed when building the application. The po filenames must be in the form 'es_ES.po' or 'es.po' (without quotes).

To create a DEB package use CPack.

Javier Campón Pichardo
