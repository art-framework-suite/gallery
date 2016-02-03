# project name
project(FWLite)

include(ArtMake)

set(canvas_FWLite_sources
  BranchData.cc
  BranchMapReader.cc
  DataGetterHelper.cc
  Event.cc
  EventHistoryGetter.cc
  EventNavigator.cc
  HistoryGetterBase.cc
  InputTag.cc
  throwFunctions.cc
)

set( canvas_FWLite_libraries
        canvas_Utilities
        canvas_Persistency_Common
        canvas_Persistency_Provenance
        ${ROOT_CORE}
        ${ROOT_RIO}
        ${ROOT_TREE}
)

art_make_library( LIBRARY_NAME canvas_FWLite
                  SOURCE ${canvas_FWLite_sources}
                  LIBRARIES ${canvas_FWLite_libraries}
)

art_dictionary(DICTIONARY_LIBRARIES canvas_FWLite)

install_headers()
install_source()
