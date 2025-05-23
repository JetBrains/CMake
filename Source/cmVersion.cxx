/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmVersion.h"

#include "cmVersionConfig.h"

unsigned int cmVersion::GetMajorVersion()
{
  return CMake_VERSION_MAJOR;
}
unsigned int cmVersion::GetMinorVersion()
{
  return CMake_VERSION_MINOR;
}
unsigned int cmVersion::GetPatchVersion()
{
  return CMake_VERSION_PATCH;
}
unsigned int cmVersion::GetTweakVersion()
{
  return 0;
}

char const* cmVersion::GetCMakeVersion()
{
  return CMake_VERSION;
}

char const* cmVersion::GetCMakeVersionExtra()
{
  return CMake_VERSION_EXTRA;
}
