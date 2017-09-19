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


# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)

project(gallery)

# cetbuildtools contains our cmake modules
find_package(cetbuildtools REQUIRED)

list(APPEND CMAKE_MODULE_PATH $ENV{CANVAS_ROOT_IO_DIR}/Modules)

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
  EXTRA_CXX_FLAGS -Wnon-virtual-dtor -Wdelete-non-virtual-dtor)
cet_have_qual(sse2 SSE2)
if (SSE2)
  cet_add_compiler_flags(CXX -msse2 -ftree-vectorizer-verbose=2)
endif()

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product(canvas_root_io)
find_ups_product(canvas)
find_ups_product(messagefacility)
find_ups_product(fhiclcpp)
find_ups_product(cetlib v2_01_00)
find_ups_product(cetlib_except v1_01_00)
find_ups_product(cetbuildtools)
find_ups_product(clhep)
find_ups_boost()
find_ups_root()

# art_dictionary
include(ArtDictionary)

# source
add_subdirectory(gallery)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
