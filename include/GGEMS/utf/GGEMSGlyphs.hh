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

  char32_t minus;
  char32_t plus;

  char32_t arrow;
  char32_t sub_arrow;

  char32_t pulse1;
  char32_t pulse2;
  char32_t pulse3;

  char32_t separator;
  char32_t title_bar;

  char32_t block_filled;
  char32_t block_empty;

  char32_t border_top_left;
  char32_t border_top_right;
  char32_t border_bottom_left;
  char32_t border_bottom_right;

  char32_t horizontal_line;
  char32_t vertical_line;
  char32_t border_rigth;
  char32_t border_left;
};

inline const GlyphSet Ascii{U'g', U'e', U'p', U'l', U'a', U'n', U'-',
                            U'+', U'-', U'*', U'*', U'.', U'$', U'-',
                            U'=', U'#', U'-', U'+', U'+', U'+', U'+',
                            U'*', U'*', U'+', U'+'};

inline const GlyphSet Utf32{U'γ', U'e', U'p', U'λ', U'α', U'ν', U'⁻',
                            U'⁺', U'→', U'↳', U'✶', U'•', U'✸', U'─',
                            U'≡', U'█', U'░', U'╔', U'╗', U'╚', U'╝',
                            U'═', U'║', U'╟', U'╢'};

inline const GlyphSet &Glyphs() noexcept {
  if (ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
      ggems::core::Encoding::Ascii)
    return Ascii;
  else
    return Utf32;
}
} // namespace ggems::utf
