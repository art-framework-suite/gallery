#ifndef gallery_EventNavigator_h
#define gallery_EventNavigator_h

// Manages iteration over the events in a vector of input files.  It
// handles the iteration, opening the files, opening TTrees, reading
// EventAuxilliary objects and (if necessary) reading History objects
// from the input files.

#include "canvas/Persistency/Provenance/Compatibility/History.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"

#include "TFile.h"

#include <memory>
#include <string>
#include <vector>

class TBranch;
class TTree;

namespace gallery {

  class EventNavigator {
  public:
    explicit EventNavigator(std::vector<std::string> const& iFileNames);

    // In a normal iteration using the next function, isValid and
    // atEnd will always return opposite values. If nextFile was
    // called directly and there was an empty file, they might
    // return different values.

    bool
    isValid() const
    {
      return fileEntry_ != numberOfFiles_ &&
             eventEntry_ != entriesInCurrentFile_;
    }

    bool
    atEnd() const
    {
      return fileEntry_ == numberOfFiles_;
    }

    void toBegin();
    void next();
    void previous();
    void goToEntry(long long entry);
    void nextFile();

    art::EventAuxiliary const& eventAuxiliary() const;
    art::ProcessHistoryID const& processHistoryID() const;
    art::ProcessHistory const& processHistory() const;

    TFile* getTFile() const;
    TTree* getTTree() const;
    TBranch*
    eventAuxiliaryBranch() const
    {
      return eventAuxiliaryBranch_;
    }

    long long
    fileEntry() const
    {
      return fileEntry_;
    }
    long long
    entriesInCurrentFile() const
    {
      return entriesInCurrentFile_;
    }
    long long
    eventEntry() const
    {
      return eventEntry_;
    }

  private:
    art::History const& history() const;
    void initializeTTreePointers();
    void initializeTBranchPointers();

    std::vector<std::string> fileNames_;
    long long numberOfFiles_;
    long long fileEntry_{-1};
    long long firstFileWithEvent_{};

    long long entriesInCurrentFile_{};
    long long eventEntry_{};

    std::unique_ptr<TFile> file_{nullptr};

    TTree* eventsTree_{nullptr};
    TBranch* eventAuxiliaryBranch_{nullptr};
    mutable art::EventAuxiliary eventAuxiliary_{};
    art::EventAuxiliary* pEventAuxiliary_{&eventAuxiliary_};
    mutable long long previousEventAuxiliaryEntry_{-1};

    TTree* eventHistoryTree_{nullptr};
    TBranch* eventHistoryBranch_{nullptr};
    art::History eventHistory_{};
    art::History* pEventHistory_{&eventHistory_};
    mutable long long previousEventHistoryEntry_{-1};

    art::FileFormatVersion fileFormatVersion_{};
    art::ProcessHistoryMap historyMap_{};
  };
} // namespace gallery

#endif /* gallery_EventNavigator_h */

// Local Variables:
// mode: c++
// End:
