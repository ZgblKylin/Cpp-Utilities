#ifndef UTILITIES_COMMON_HPP
#define UTILITIES_COMMON_HPP

/**
 * \mainpage Utilities
 *
 * \section repository_introduction Introduction
 * The Utilities repository provides many useful functionalities.\n
 * All codes are written header-only, so no compile-export-import-link is needed,
 * just use `git-submodule` to add to your project and include whatever `.hpp`
 * files you need.
 *
 * \section repository_modules Modules
 * \ref DimensionalAnalysis \n
 * Helpr classes, typedefs and functions for dimensional analyse.
 *
 * \section repository_files Files
 * - \ref Common.h Macros defined for utilities.
 * - \ref DimensionalAnalysis/
 *   - \ref DimensionalAnalysis.hpp Functionalities for dimensional analysis,
 *     guaranteed by strong type, and provides zero-cost abstraction.
 *   - \ref Ratios.hpp Functionalities for ratio calculation, such as
 *     generating approximiate fraction from decimals.
 */

/**
 * \file Common.h
 * \brief Macros defined for utilities.
 * \details
 * This file provides macros defined for all utilites.\n
 * If macro `UTILITIES_NAMESPACE` is defined to a specific symbol name, all
 * utility symbols will be defined in this namespace.\n
 * **Example:**
 * ```cpp
 * #define UTILITIES_NAMESPACE Utilities
 * ```
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
#  define UTILITIES_NAMESPACE_BEGIN namespace UTILITIES_NAMESPACE {
#  define UTILITIES_NAMESPACE_END }
#else
#  define UTILITIES_NAMESPACE_BEGIN
#  define UTILITIES_NAMESPACE_END
#endif

#endif // UTILITIES_COMMON_HPP
