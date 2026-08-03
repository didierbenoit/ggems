#pragma once

#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::utf {

struct GlyphSet {
  char32_t gamma;
  char32_t electron;
  char32_t proton;
  char32_t aionino;
  char32_t alpha;
  char32_t neutron;
  char32_t mu;
  char32_t minus;
  char32_t plus;
  char32_t pulse2;
  char32_t separator;
  char32_t block_filled;
  char32_t border_top_left;
  char32_t border_top_right;
  char32_t border_bottom_left;
  char32_t border_bottom_right;
  char32_t horizontal_line;
  char32_t vertical_line;
  char32_t border_right;
  char32_t border_left;
  char32_t superscript_two;
  char32_t superscript_three;
};

inline const GlyphSet Ascii{.gamma = U'g',
                            .electron = U'e',
                            .proton = U'p',
                            .aionino = U'l',
                            .alpha = U'a',
                            .neutron = U'n',
                            .mu = U'u',
                            .minus = U'-',
                            .plus = U'+',
                            .pulse2 = U'.',
                            .separator = U'-',
                            .block_filled = U'#',
                            .border_top_left = U'+',
                            .border_top_right = U'+',
                            .border_bottom_left = U'+',
                            .border_bottom_right = U'+',
                            .horizontal_line = U'*',
                            .vertical_line = U'*',
                            .border_right = U'+',
                            .border_left = U'+',
                            .superscript_two = U'2',
                            .superscript_three = U'3'};

inline const GlyphSet Utf32{.gamma = U'γ',
                            .electron = U'β',
                            .proton = U'p',
                            .aionino = U'λ',
                            .alpha = U'α',
                            .neutron = U'ν',
                            .mu = U'\u00B5',
                            .minus = U'-',
                            .plus = U'+',
                            .pulse2 = U'•',
                            .separator = U'─',
                            .block_filled = U'█',
                            .border_top_left = U'╔',
                            .border_top_right = U'╗',
                            .border_bottom_left = U'╚',
                            .border_bottom_right = U'╝',
                            .horizontal_line = U'═',
                            .vertical_line = U'║',
                            .border_right = U'╟',
                            .border_left = U'╢',
                            .superscript_two = U'\u00B2',
                            .superscript_three = U'\u00B3'};

inline auto Glyphs() noexcept -> GlyphSet const & {
  if (ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
      ggems::core::Encoding::Ascii) {
    return Ascii;
  }

  return Utf32;
}
} // namespace ggems::utf
