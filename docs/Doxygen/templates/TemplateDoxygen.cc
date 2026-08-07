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
 * \file XXXX
 * \brief Base exception class and specialized GGEMS exception categories.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */


namespace ggems::core {

// ============================================================================
// ============================================================================

void GGEMSExceptionBase::Log(std::string const &msg) noexcept {}

// ----------------------------------------------------------------------------

void GGEMSExceptionBase::Log2(std::string const &msg) noexcept {}

// ============================================================================
// ============================================================================

void TerminateHandler() noexcept {}
} // namespace ggems::core
