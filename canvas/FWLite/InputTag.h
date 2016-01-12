#ifndef art_FWLite_InputTag_h
#define art_FWLite_InputTag_h

// Holds module label, product instance, and process strings
// which are passed when requesting a data product.

#include <iosfwd>
#include <string>

namespace canvas {

  class InputTag {
  public:

    InputTag();

    // Create an InputTag by parsing the given string, which is
    // expected to be in colon-delimited form, fitting one of the
    // following patterns:
    //
    //   label
    //   label:instance
    //   label:instance:process
    InputTag(std::string const& s);
    InputTag(char const * s);

    // Create an InputTag with the given label, instance, and process
    // specificiations.
    InputTag(std::string const& label,
             std::string const& instance,
             std::string const& processName = std::string());

    // Create an InputTag with the given label, instance, and process
    // specifications.
    InputTag(char const* label,
             char const* instance,
             char const* processName="");


    // use compiler-generated copy c'tor, copy assignment, and d'tor

    std::string const& label() const {return label_;}
    std::string const& instance() const {return instance_;}
    ///an empty string means find the most recently produced
    ///product with the label and instance
    std::string const& process() const {return process_;}

    bool operator==(InputTag const& tag) const;

  private:

    std::string label_;
    std::string instance_;
    std::string process_;

    // Helper function, to parse colon-separated initialization
    // string.
    void set_from_string_(std::string const& s);
  };

  std::ostream& operator << (std::ostream& ost, InputTag const& tag);
  bool operator != (InputTag const & left, InputTag const & right);
}

#endif /* art_FWLite_InputTag_h */

// Local Variables:
// mode: c++
// End:
