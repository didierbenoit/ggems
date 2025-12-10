#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSWindowsCore.hh
 * \brief Core Windows platform definitions and minimal system includes required
 * by GGEMS.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright GNU GPL v3
 *
 * This header centralises very light-weight Windows-specific system includes
 * employed by GGEMS to query OS state, access process information, and interact
 * with device and memory subsystems under Microsoft Windows.
 *
 * The goal is to confine these low-level dependencies to a single location
 * in order to prevent namespace pollution and ensure clear separation between
 * platform logic and simulation logic.
 */

#if defined(_WIN32)
/*!
 * \def WIN32_LEAN_AND_MEAN
 * \brief Reduces the size of the Windows header footprint by excluding rarely
 * used APIs.
 */
#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
/*!
 * \def NOMINMAX
 * \brief Prevents definition of min/max macros from the Windows headers to
 * avoid conflicts.
 */
#define NOMINMAX
#endif

/// \cond
#include <io.h>
#include <windows.h>
#include <winternl.h>
#include <Psapi.h>
/// \endcond

/*!
 * \def isatty
 * \brief Maps POSIX function name `isatty` to the Windows CRT equivalent
 * `_isatty`.
 */
#define isatty _isatty

/*!
 * \def fileno
 * \brief Maps POSIX function name `fileno` to the Windows CRT equivalent
 * `_fileno`.
 */
#define fileno _fileno
#endif
