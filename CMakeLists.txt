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
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()
set(CMAKE_MODULE_PATH $ENV{CANVAS_DIR}/Modules 
                      $ENV{CETBUILDTOOLS_DIR}/Modules 
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
  )
cet_have_qual(sse2 SSE2)
if ( SSE2 )
  cet_add_compiler_flags(CXX -msse2 -ftree-vectorizer-verbose=2)
endif()

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( canvas )
find_ups_product ( cetbuildtools )

# macros for art_dictionary and simple_plugin
include(ArtDictionary)
include(BuildPlugins)

# source
add_subdirectory (gallery)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
