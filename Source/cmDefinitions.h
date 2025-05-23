/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <unordered_map>
#include <vector>

#include <cm/string_view>

#include "cmLinkedTree.h"
#include "cmString.hxx"
#include "cmValue.h"

namespace Sysprogs
{
class HLDPServer;
}

/** \class cmDefinitions
 * \brief Store a scope of variable definitions for CMake language.
 *
 * This stores the state of variable definitions (set or unset) for
 * one scope.  Sets are always local.  Gets search parent scopes
 * transitively and save results locally.
 */
class cmDefinitions
{
  using StackIter = cmLinkedTree<cmDefinitions>::iterator;
  friend class Sysprogs::HLDPServer;
  
public:
  // -- Static member functions

  static cmValue Get(std::string const& key, StackIter begin, StackIter end);

  static void Raise(std::string const& key, StackIter begin, StackIter end);

  static bool HasKey(std::string const& key, StackIter begin, StackIter end);

  static std::vector<std::string> ClosureKeys(StackIter begin, StackIter end);

  static cmDefinitions MakeClosure(StackIter begin, StackIter end);

  // -- Member functions

  /** Set a value associated with a key.  */
  void Set(std::string const& key, cm::string_view value);

  /** Unset a definition.  */
  void Unset(std::string const& key);

private:
  /** String with existence boolean.  */
  struct Def
  {
  public:
    Def() = default;
    Def(cm::string_view value)
      : Value(value)
    {
    }
    cm::String Value;
  };
  static Def NoDef;

  std::unordered_map<cm::String, Def> Map;

  static Def const& GetInternal(std::string const& key, StackIter begin,
                                StackIter end, bool raise);
};
