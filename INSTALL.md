DOSBoxGTK - INSTALL
===================

The DOSBoxGTK project uses CMake and has some custom targets to make things
easier.
To create a Release version set CMAKE_BUILD_TYPE CMake variable to RELEASE.
To create a Debug version just don't set the CMAKE_BUILD_TYPE variable or set
it to DEBUG.

Custom targets:
---------------
- doc: use make doc to generate the Doxygen documentation.
- pot: use make pot to generate the PO Template file.
- install: use make install to install the application (only available on
  Release build type)
- uninstall: use make uninstall to remove the previously installed application
  (only available on Release build type. Will need the install_manifest.txt
  file created on installation.)

Any available po file in the translations dir will be automatically processed
when building the application. The po filenames must be in the form 'es_ES.po'
or 'es.po' (without quotes).

                                                          Javier Campón Pichardo
