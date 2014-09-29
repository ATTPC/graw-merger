# README #

This tool can be used to convert the GRAW output files to a more compact, single-file, event-based format.

### Compiling ###

The project can either be built using the included Xcode project or by using GNU autotools as described in INSTALL.

Dependencies:

* Boost libraries, latest version. Download from http://www.boost.org/ and compile at least the System, Program Options, and Filesystem libraries. The Xcode project will expect to find boost headers in /usr/local/include and compiled static libraries in /usr/local/lib.

* Google Test and Google Mock frameworks, available at https://code.google.com/p/googlemock/. These are only necessary to build the unit testing targets.

* Doxygen (http://www.doxygen.org), Sphinx (http://sphinx-doc.org), and Breathe (http://michaeljones.github.io/breathe) for building the documentation. If you don't want to install these, remove the "Run Script" build phase in Xcode. (This is not relevant if building with autotools.)