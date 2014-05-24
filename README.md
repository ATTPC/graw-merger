# README #

This tool can be used to convert the 40 AT-TPC GRAW output files to a more compact, single-file format.

### Compiling ###

The Xcode project for this program is in the repository. Just open it and build any of the schemes that start with "GETConsolidate."

There are, however, some dependencies that need to be satisfied.

* Boost libraries, latest version. Download from http://www.boost.org/ and compile at least the System, Program Options, and Filesystem libraries. The Xcode project will expect to find boost headers in /usr/local/include and compiled static libraries in /usr/local/lib.

* Google Test and Google Mock frameworks, available at https://code.google.com/p/googlemock/. These are only necessary to build the unit testing targets.

* Doxygen (http://www.doxygen.org), Sphinx (http://sphinx-doc.org), and Breathe (http://michaeljones.github.io/breathe) for building the documentation. If you don't want to install these, remove the "Run Script" build phase in Xcode.