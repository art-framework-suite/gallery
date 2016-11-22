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
  HistoryGetterBase.cc
  throwFunctions.cc
  ValidHandle.cc
)

set(gallery_libraries
  canvas_Persistency_Common
  canvas_Persistency_Provenance
  canvas_Utilities
  cetlib
  cetlib_except
  ${ROOT_CORE}
  ${ROOT_RIO}
  ${ROOT_TREE}
  )

cet_make(LIBRARIES
  canvas_Persistency_Common
  canvas_Persistency_Provenance
  canvas_Utilities
  cetlib
  cetlib_except
  ${ROOT_CORE}
  ${ROOT_RIO}
  ${ROOT_TREE}
  NO_DICTIONARY
)

art_dictionary(DICTIONARY_LIBRARIES gallery)

install_headers()
install_source()
