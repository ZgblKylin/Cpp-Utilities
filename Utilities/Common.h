#ifndef UTILITIES_COMMON_HPP
#define UTILITIES_COMMON_HPP

/**
 * \mainpage Utilities
 *
 * \section repository_introduction Introduction
 * The Utilities repository provides many useful functionalities.\n
 * All codes are written header-only, so no compile-export-import-link is
 * needed, just use `git-submodule` to add to your project and include whatever
 * `.hpp` files you need.
 *
 * \section repository_modules Modules
 * \ref DimensionalAnalysis \n
 * Helpr classes, typedefs and functions for dimensional analyse.\n
 * \ref MemorySafety\n
 * Helper classes, typedefs and functions for memroy safety.
 *
 * \section repository_files Files
 * - \ref Common.h Macros defined for utilities.
 * - DimensionalAnalysis/
 *   - \ref DimensionalAnalysis.hpp Functionalities for dimensional analysis,
 *      guaranteed by strong type, and provides zero-cost abstraction.
 *   - \ref Ratios.hpp Functionalities for ratio calculation, such as
 *     generating approximiate fraction from decimals.
 * - MemorySafety/
 *   - \ref RWSpinLock.hpp A extremely high-performance read-write-spinlock
 *     imported from folly library.
 *   - \ref SafeShardPtr.hpp Classes wrapped from `std::shared_ptr` /
 *     `std::weak_ptr` and `std::enable_shared_from_this` to provide thread-safety
 *     while operating the underlying pointer.
 */

/**
 * \file Common.h
 * \brief Macros defined for utilities.
 * \details
 *   This file provides macros defined for all utilites.\n
 *   If macro `UTILITIES_NAMESPACE` is defined to a specific symbol name, all
 *   utility symbols will be defined in this namespace.\n
 *   **Example:**
 *   ```cpp
 *   #define UTILITIES_NAMESPACE Utilities
 *   ```
 */

/**
 * \def UTILITIES_USING_NAMESPACE
 * \brief Define for using namespace declaration, nothing will be generated if
 *        `UTILITIES_NAMESPACE` isn't defined.
 */

/**
 * \def UTILITIES_NAMESPACE_PREFIX
 * \brief Define for namespace:: declaration, nothing will be generated if
 *        `UTILITIES_NAMESPACE` isn't defined.
 */

/**
 * \def UTILITIES_NAMESPACE_BEGIN
 * \brief Define for begin namespace declaration, nothing will be generated if
 *        `UTILITIES_NAMESPACE` isn't defined.
 */

/**
 * \def UTILITIES_NAMESPACE_END
 * \brief Define for end namespace declaration, nothing will be generated if
 *        `UTILITIES_NAMESPACE` isn't defined.
 */

#ifdef UTILITIES_NAMESPACE
#  define UTILITIES_USING_NAMESPACE using namespace UTILITIES_NAMESPACE
#  define UTILITIES_NAMESPACE_PREFIX UTILITIES_NAMESPACE::
#  define UTILITIES_NAMESPACE_BEGIN namespace UTILITIES_NAMESPACE {
#  define UTILITIES_NAMESPACE_END }
#else
#  define UTILITIES_USING_NAMESPACE
#  define UTILITIES_NAMESPACE_PREFIX
#  define UTILITIES_NAMESPACE_BEGIN
#  define UTILITIES_NAMESPACE_END
#endif

#endif // UTILITIES_COMMON_HPP
