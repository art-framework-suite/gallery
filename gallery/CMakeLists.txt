include(CetMake)

find_package(Range-v3 REQUIRED)
find_package(ROOT COMPONENTS Core RIO Tree REQUIRED)
find_package(cetlib REQUIRED EXPORT)
find_package(canvas REQUIRED EXPORT)
find_package(canvas_root_io REQUIRED EXPORT)

include(ArtDictionary)

cet_make_library(HEADERS_TARGET
  SOURCE
    AssnsBranchData.cc
    BranchData.cc
    BranchMapReader.cc
    DataGetterHelper.cc
    InfoForTypeLabelInstance.cc
    Event.cc
    EventHistoryGetter.cc
    EventNavigator.cc
    FindMaker.cc
    throwFunctions.cc
    ValidHandle.cc
  LIBRARIES
  PUBLIC
    canvas_root_io::canvas_root_io
    canvas::canvas
    cetlib::headers
    cetlib::container_algorithms
    ROOT::RIO
  PRIVATE
    ROOT::Tree
    ROOT::Core
    range-v3::range-v3
  )

art_dictionary(DICTIONARY_LIBRARIES gallery::gallery)

if (BUILD_TESTING)
  add_subdirectory(test)
endif()

install_headers()
install_source()
