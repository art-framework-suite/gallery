# ======================================================================
#  gallery main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/gallery/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package (builds distribution tarfile)
# ======================================================================

cmake_minimum_required(VERSION 3.12...3.14 FATAL_ERROR)
project(gallery LANGUAGES CXX)

project(gallery VERSION 1.12.03 LANGUAGES CXX)

# cetmodules contains our cmake modules
find_package(cetmodules REQUIRED)

list(APPEND CMAKE_MODULE_PATH $ENV{CANVAS_ROOT_IO_DIR}/Modules)

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
  EXTRA_CXX_FLAGS -Wnon-virtual-dtor -Wdelete-non-virtual-dtor)

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_package(canvas REQUIRED)
find_package(canvas_root_io REQUIRED)
find_package(cetlib REQUIRED)
find_package(cetlib_except REQUIRED)

# art_dictionary
include(ArtDictionary)

# source
add_subdirectory(gallery)

# ups - table and config files
add_subdirectory(cmakeConfig)
