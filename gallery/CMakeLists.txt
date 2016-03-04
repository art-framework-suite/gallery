
include(ArtMake)

set(gallery_sources
  BranchData.cc
  BranchMapReader.cc
  DataGetterHelper.cc
  Event.cc
  EventHistoryGetter.cc
  EventNavigator.cc
  Handle.cc
  HistoryGetterBase.cc
  throwFunctions.cc
  ValidHandle.cc
)

set( gallery_libraries
        canvas_Persistency_Common
        canvas_Persistency_Provenance
        canvas_Utilities
        cetlib
        ${ROOT_CORE}
        ${ROOT_RIO}
        ${ROOT_TREE}
)

art_make_library( LIBRARY_NAME gallery
                  SOURCE ${gallery_sources}
                  LIBRARIES ${gallery_libraries}
)

art_dictionary(DICTIONARY_LIBRARIES gallery)

install_headers()
install_source()
