
#include "gallery/InputTag.h"
#include "canvas/Utilities/Exception.h"

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/constants.hpp"
#include "boost/algorithm/string/split.hpp"

#include <ostream>
#include <vector>

namespace gallery {

  InputTag::InputTag() :
    label_(),
    instance_(),
    process_()
  {
  }

  InputTag::InputTag(std::string const& s) :
    label_(),
    instance_(),
    process_()
  {
    set_from_string_(s);
  }

  InputTag::InputTag(const char* s) :
    label_(),
    instance_(),
    process_()
  {
    set_from_string_(s);
  }

  InputTag::InputTag(std::string const& label,
                     std::string const& instance,
                     std::string const& processName) :
    label_(label),
    instance_(instance),
    process_(processName)
  {
  }

  InputTag::InputTag(char const* label,
                     char const* instance,
                     char const* processName) :
    label_(label),
    instance_(instance),
    process_(processName)
  {
  }

  bool InputTag::operator==(InputTag const& tag) const
  {
    return (label_ == tag.label_)
        && (instance_ == tag.instance_)
        && (process_ == tag.process_);
  }

  bool operator!=(InputTag const & left, InputTag const & right)
  {
    return ! (left == right);
  }

  void InputTag::set_from_string_(std::string const& s)
  {
    // string is delimited by colons
    std::vector<std::string> tokens;
    // cet::split(s, ':', std::back_inserter(tokens));
    boost::split(tokens, s, boost::is_any_of(":"), boost::token_compress_off);

    int nwords = tokens.size();
    if(nwords > 3) {
      throw art::Exception(art::errors::Configuration,"InputTag")
        << "Input tag " << s << " has " << nwords << " tokens";
    }
    if(nwords > 0) label_ = tokens[0];
    if(nwords > 1) instance_ = tokens[1];
    if(nwords > 2) process_=tokens[2];
  }

  std::ostream& operator<<(std::ostream& ost, InputTag const& tag) {
    static std::string const process(", process = ");
    ost << "InputTag:  label = " << tag.label()
        << ", instance = " << tag.instance()
        <<(tag.process().empty()?std::string():(process+tag.process()));
    return ost;
  }

}
