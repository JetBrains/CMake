/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

/* Use the cppdap library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_CPPDAP
#  include <dap/protocol.h> // IWYU pragma: export
#else
#  include <cmcppdap/include/dap/protocol.h> // IWYU pragma: export
#endif
