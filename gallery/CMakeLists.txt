include(CetMake)

cet_make_library(
  LIBRARY_NAME gallery
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
    canvas_root_io::canvas_root_io
  PRIVATE
    ROOT::Core
    ROOT::RIO
    ROOT::Tree
  )

art_dictionary(DICTIONARY_LIBRARIES PRIVATE gallery::gallery)

add_subdirectory(test)

install_headers()
install_source()
