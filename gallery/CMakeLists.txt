include(CetMake)

set(gallery_sources
  AssnsBranchData.cc
  BranchData.cc
  BranchMapReader.cc
  DataGetterHelper.cc
  Event.cc
  EventHistoryGetter.cc
  EventNavigator.cc
  FindMaker.cc
  throwFunctions.cc
  ValidHandle.cc
)

set(gallery_libraries
  canvas
  canvas_root_io
  fhiclcpp
  cetlib
  cetlib_except
  ${ROOT_CORE}
  ${ROOT_RIO}
  ${ROOT_TREE}
  )

cet_make(LIBRARIES
  ${gallery_libraries}
  NO_DICTIONARY
  )

art_dictionary(DICTIONARY_LIBRARIES gallery)

add_subdirectory(test)

install_headers()
install_source()
