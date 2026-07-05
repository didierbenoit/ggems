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
 * \file GGEMSGlyphs.hh
 * \brief UTF-32 glyph set for ASCII and Unicode UI rendering in GGEMS.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright GNU GPL v3
 *
 * This header defines a fixed bundle of UTF-32 glyphs used throughout the
 * GGEMS user interface. The glyph set abstracts symbols used in terminal
 * rendering, progress bars, UI borders, particle tags, decorative characters,
 * and other graphical hints. Two fixed variants are exposed:
 *
 *  - ASCII fallback: composed only of 7-bit printable characters, ensuring
 *    safe output on legacy or restricted terminals.
 *  - UTF-32 rich set: providing higher-fidelity graphical elements including
 *    arrows, border lines, Greek symbols, blocks, and various UI motifs.
 *
 * At runtime, the active glyph set is selected based on the encoding state
 * stored in the logger subsystem. This provides transparent adaptation to
 * user environments while maintaining formatting consistency across GGEMS
 * components.
 */

#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::utf {

/*!
 * \struct GlyphSet
 * \brief Bundle of UI glyphs encoded as UTF-32 code points.
 *
 * Each member identifies a specific UI symbol. These symbols are used for
 * particle icons (electron, proton, gamma), terminal separators, progress
 * bars, decorative marks, and UTF-compatible border layouts.
 *
 * Instances of \ref GlyphSet are expected to be statically initialised and
 * immutable. The static variants defined in this header cover ASCII fallback
 * and UTF-32 rich modes.
 */
struct GlyphSet {
  char32_t gamma;               /*!< Gamma particle symbol */
  char32_t electron;            /*!< Electron particle symbol */
  char32_t proton;              /*!< Proton particle symbol */
  char32_t aionino;             /*!< Internal mascot marker ("aionino") */
  char32_t alpha;               /*!< Greek alpha symbol */
  char32_t neutron;             /*!< Neutron symbol */
  char32_t minus;               /*!< Minus operator for UI elements */
  char32_t plus;                /*!< Plus operator for UI elements */
  char32_t arrow;               /*!< Main arrow symbol */
  char32_t sub_arrow;           /*!< Secondary arrow symbol */
  char32_t pulse1;              /*!< Pulse symbol #1 (progress visuals) */
  char32_t pulse2;              /*!< Pulse symbol #2 */
  char32_t pulse3;              /*!< Pulse symbol #3 */
  char32_t separator;           /*!< Separator used in logs and menus */
  char32_t title_bar;           /*!< Title bar symbol for headers */
  char32_t block_filled;        /*!< Solid block for fill/UI geometry */
  char32_t block_empty;         /*!< Empty block variant */
  char32_t border_top_left;     /*!< Top-left border corner */
  char32_t border_top_right;    /*!< Top-right border corner */
  char32_t border_bottom_left;  /*!< Bottom-left border corner */
  char32_t border_bottom_right; /*!< Bottom-right border corner */
  char32_t horizontal_line;     /*!< Horizontal border segment */
  char32_t vertical_line;       /*!< Vertical border segment */
  char32_t border_right;        /*!< Mid-edge connector on the right */
  char32_t border_left;         /*!< Mid-edge connector on the left */
};

/*!
 * \brief ASCII fallback glyph set.
 *
 * Composed of plain printable characters, ensuring guaranteed display on
 * terminals lacking full Unicode compatibility.
 */
inline const GlyphSet Ascii{U'g', U'e', U'p', U'l', U'a', U'n', U'-',
                            U'+', U'-', U'*', U'*', U'.', U'$', U'-',
                            U'=', U'#', U'-', U'+', U'+', U'+', U'+',
                            U'*', U'*', U'+', U'+'};

/*!
 * \brief UTF-32 rich glyph set.
 *
 * Includes higher fidelity line art, Greek letters, directional arrows,
 * box-drawing characters, and decorative particles used in GGEMS terminals
 * and framebuffers.
 */
inline const GlyphSet Utf32{U'γ', U'β', U'p', U'λ', U'α', U'ν', U'-',
                            U'+', U'→', U'↳', U'✶', U'•', U'✸', U'─',
                            U'≡', U'█', U'░', U'╔', U'╗', U'╚', U'╝',
                            U'═', U'║', U'╟', U'╢'};

/*!
 * \brief Return the active glyph set based on engine encoding mode.
 *
 * This accessor inspects the encoding configuration from the main logger.
 * If ASCII output is explicitly requested, the fallback ASCII set is
 * returned. Otherwise, the UTF-32 rich set is selected.
 *
 * \return Reference to either \ref ggems::utf::Ascii or \ref ggems::utf::Utf32.
 */
inline const GlyphSet &Glyphs() noexcept {
  if (ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
      ggems::core::Encoding::Ascii)
    return Ascii;
  else
    return Utf32;
}
} // namespace ggems::utf
