#ifndef gallery_DataGetterHelper_h
#define gallery_DataGetterHelper_h

// Allows one to lookup a product given module label, instance name,
// type and optionally the process name. It contains various data
// structures to efficiently perform that lookup.  Using
// BranchMapReader it can also find a product using the ProductID
// stored in Ptrs.

// By default TTreeCache is used and it is the design intent that the
// configuration defaults are reasonably good and the user needs to do
// nothing. in what follows there are some details of how it works and
// some alternatives, but most people can stop reading this comment
// here and not waste their time ... The default cache sizes of ROOT
// (currently 30 MB)is used. This code decides which branches to put
// into the cache as follows. It includes all branches where the
// type/label/instance was requested and the process both exists in
// some product registry in an input file already open and the branch
// exists in the currently open input file. It initializes the cache
// when the specified number of events has been processed
// (eventsToLearnUsedBranches argument of the Event constructor which
// defaults to 7). And also everytime a new file is opened after that
// point. The user has some alternatives. By using the second argument
// of the Event contructor, one could turn off TTreeCache usage by
// this code and run without a TTreeCache. Alternately, one could turn
// it off and get the TTree* at each file open and manually configure
// the TTreeCache however one would like. Note EventAuxiliary is
// special in that it is always cached. Also note that in the
// implementation TChain is not used so if one were to manually
// configure one would have to do that everytime a new TFile was
// opened which could be detected by looking for a change in the file
// entry number using the Event interface.

#include "gallery/BranchData.h"
#include "gallery/BranchMapReader.h"
#include "gallery/EventHistoryGetter.h"
#include "gallery/TypeLabelInstanceKey.h"

#include "canvas/Persistency/Common/EDProductGetterFinder.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas_root_io/Utilities/DictionaryChecker.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

namespace art {
  class EDProduct;
  class EDProductGetter;
  class BranchDescription;
  class ProductID;
} // namespace art

class TClass;
class TFile;
class TTree;

namespace gallery {

  class EventNavigator;

  class DataGetterHelper : art::EDProductGetterFinder {
  public:
    DataGetterHelper(EventNavigator const* eventNavigator,
                     std::shared_ptr<EventHistoryGetter> historyGetter);

    void getByLabel(std::type_info const& typeInfoOfWrapper,
                    art::InputTag const& inputTag,
                    art::EDProduct const*& edProduct) const;

    void updateFile(TFile* iFile, TTree* iTree, bool initializeTheCache);
    void initializeTTreeCache();
    void updateEvent();

  private:
    using uupair = std::pair<unsigned int, unsigned int>;

    EventNavigator const* eventNavigator_;
    TTree* tree_{nullptr};
    std::shared_ptr<EventHistoryGetter> historyGetter_;
    mutable bool initializedForProcessHistory_{false};
    mutable art::ProcessHistoryID previousProcessHistoryID_{};
    mutable std::vector<std::string> previousProcessHistoryNames_{};

    // Includes all processNames seen in ProcessHistories.  They are
    // pushed onto the back of the vector when first seen and then
    // never modified or moved.
    mutable std::vector<std::string> processNames_{};

    // The key to this map is the process name. The value is the index
    // into the vector processNames_
    mutable std::map<std::string, unsigned int> processNameToProcessIndex_{};

    // These are ordered by the current ProcessHistory
    mutable std::vector<unsigned int> orderedProcessIndexes_{};

    // One of these info objects is created the first time each unique
    // key consisting of the type, module label, and instance is used
    // in a call to getByLabel, getValidHandle, or a similar function.
    class InfoForTypeLabelInstance {
    public:
      InfoForTypeLabelInstance(art::TypeID const& iType,
                               std::string const& iLabel,
                               std::string const& iInstance);

      art::TypeID const&
      type() const
      {
        return type_;
      }
      std::string const&
      label() const
      {
        return label_;
      }
      std::string const&
      instance() const
      {
        return instance_;
      }

      TClass*
      tClass() const
      {
        return tClass_;
      }
      bool
      isAssns() const
      {
        return isAssns_;
      }
      art::TypeID const&
      partnerType() const
      {
        return partnerType_;
      }

      std::vector<std::pair<unsigned int, unsigned int>>&
      processIndexToBranchDataIndex() const
      {
        return processIndexToBranchDataIndex_;
      }

      std::vector<unsigned int>&
      branchDataIndexOrderedByHistory() const
      {
        return branchDataIndexOrderedByHistory_;
      }

      std::vector<art::ProductID>&
      productIDs() const
      {
        return productIDs_;
      }

    private:
      art::TypeID const type_;
      std::string const label_;
      std::string const instance_;

      TClass* const tClass_;
      bool const isAssns_;
      art::TypeID const partnerType_;

      // There is an entry here for each process with a branch that is
      // in the ProductRegistry and in the input file for any input
      // file that contains events and was opened so far and the
      // product registry entry must be associated with the Event and
      // have a valid ProductID (the corresponding TBranch does not
      // necessarily need to exist in the current input file).  These
      // are maintained in the order of the processIndex. The second
      // part of the pair is an index into branchDataVector_.
      mutable std::vector<uupair> processIndexToBranchDataIndex_;

      // There is an entry here for each process in the current
      // process history with a branch in the current input ROOT file.
      // They are maintained in process history order.  The value of
      // each element is an index into the branch data vector.
      mutable std::vector<unsigned int> branchDataIndexOrderedByHistory_;

      // The ProductIDs for the processes in processNames_ This vector
      // is sorted in the same order and has the same size as
      // processNames_.
      mutable std::vector<art::ProductID> productIDs_;
    };

    // Note we just add to this, the order is never changed
    // so an index into this can be cached. Entries are only
    // added if and when data is requested via a getValidHandle
    // function call or another similar function call.
    mutable std::vector<InfoForTypeLabelInstance> infoVector_{};

    // Use this to find the desired entry in the infoVector_ using a
    // key with type, label, and instance.
    mutable std::map<TypeLabelInstanceKey, unsigned int> infoMap_{};

    // The BranchData object contains the object which ROOT fills when
    // reading the data. It also contains related items.  Note we just
    // add to this vector, the order is never changed so an index into
    // this can be cached. The TBranch* in BranchData will be set to
    // nullptr if the TBranch does not exist in the current
    // file. There are entries here when the data product is not
    // "present" because the producer declared it but never put
    // it. You have to read the data and check the isPresent bit to
    // determine that. There will be one entry in a
    // processIndexToBranchDataIndex_ that points at each element in
    // this vector. These are only created when needed (3 possible
    // things can cause new BranchData objects to be constructed,
    // 1. when a new InfoForTypeLabelInstance is constructed 2. when a
    // new process is encountered in a process history 3. when a new
    // input file is opened with new entries in its product registry).
    mutable std::vector<std::unique_ptr<BranchData>> branchDataVector_{};
    BranchData invalidBranchData_{};

    // Use this for get functions that use the ProductID. These are
    // mostly used when dereferencing a Ptr.
    mutable std::map<art::ProductID, unsigned int> branchDataIndexMap_{};
    mutable std::set<art::ProductID> branchDataMissingSet_{};

    // Keeps track of information related to metadata.
    BranchMapReader branchMapReader_{};

    mutable art::root::DictionaryChecker dictChecker_{};

    static bool streamersInitialized_;

    // ----------------- private functions -----------------------

    DataGetterHelper(DataGetterHelper const&) = delete;
    DataGetterHelper const& operator=(DataGetterHelper const&) = delete;

    static void initializeStreamers();
    void initializeForProcessHistory() const;
    void addProcess(std::string const& processName) const;
    static std::string buildBranchName(InfoForTypeLabelInstance const& info,
                                       std::string const& processName);
    void addBranchData(std::string&& branchName,
                       unsigned int processIndex,
                       InfoForTypeLabelInstance const& info,
                       bool initializeTheCache = false) const;
    TClass* getTClassUsingBranchDescription(
      unsigned int processIndex,
      InfoForTypeLabelInstance const& info) const;
    InfoForTypeLabelInstance& getInfoForTypeLabelInstance(
      art::TypeID const& type,
      std::string const& label,
      std::string const& instance) const;
    void addTypeLabelInstance(art::TypeID const& type,
                              std::string const& label,
                              std::string const& instance) const;
    void insertIntoInfoMap(art::TypeID const& type,
                           std::string const& label,
                           std::string const& instance,
                           unsigned int infoIndex) const;
    void readBranchData(unsigned int branchDataIndex,
                        art::EDProduct const*& edProduct,
                        art::TypeID const& type) const;
    bool getBranchDataIndex(
      std::vector<std::pair<unsigned int, unsigned int>> const&
        processIndexToBranchDataIndex,
      unsigned int processIndex,
      unsigned int& branchDataIndex) const;
    void updateBranchDataIndexOrderedByHistory(
      InfoForTypeLabelInstance const& info) const;
    bool getByBranchDescription(art::BranchDescription const&,
                                unsigned int& branchDataIndex) const;

    art::EDProductGetter const* getEDProductGetterImpl(
      art::ProductID) const override;
  };
} // namespace gallery
#endif /* gallery_DataGetterHelper_h */

// Local Variables:
// mode: c++
// End:
