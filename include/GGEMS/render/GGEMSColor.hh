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
 * \file GGEMSColor.hh
 * \brief Compile-time color palette and ANSI helpers for GGEMS.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright GNU GPL v3.0
 *
 * This header defines a small, deterministic color system used throughout
 * GGEMS for terminal output, framebuffer rendering, and GUI front-ends
 * (for example Vulkan/ImGui). Colors are represented as 24-bit RGB values
 * grouped into families (green, blue, red, gray, and so on) and indexed
 * by discrete shades.
 *
 * A base palette is defined for the "Normal" variant, with 12 shades per
 * family. At compile time, additional variants "Bright" and "Faint" are
 * derived through simple transformations, ensuring consistent relationships
 * between intensity levels. Color keys are then mapped to terminal
 * 24-bit ANSI escape sequences for foreground and background attributes.
 */

/// \cond
#include <array>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <cstddef>
/// \endcond

namespace ggems::render {
/*!
 * \struct RGB
 * \brief Simple 24-bit RGB color triplet.
 *
 * Each component corresponds to an 8-bit channel in the usual [0,255]
 * range. This structure acts as a low-level carrier for color data
 * and is used internally in palettes and conversions to ANSI escape
 * sequences or GUI back-ends.
 */
struct RGB {
  std::uint8_t r; /*!< Red channel in [0, 255] */
  std::uint8_t g; /*!< Green channel in [0, 255] */
  std::uint8_t b; /*!< Blue channel in [0, 255] */
};

/*!
 * \enum ColorFamily
 * \brief Logical grouping of related shades.
 *
 * A color family corresponds to a perceptual group such as "Red",
 * "Green", "Cyan", "Gray", and so on. Each family is associated with
 * 12 discrete shades in the base palette.
 */
enum class ColorFamily : std::uint8_t {
  Gray = 0, /*!< Gray-scale family */
  Red,      /*!< Red family */
  Orange,   /*!< Orange family */
  Yellow,   /*!< Yellow family */
  Green,    /*!< Green family */
  Cyan,     /*!< Cyan/Turquoise family */
  Blue,     /*!< Blue family */
  Magenta,  /*!< Magenta/Pink family */
  White,    /*!< White/near-white family */
  Count     /*!< Number of families (sentinel) */
};

/*!
 * \enum ColorVariant
 * \brief Variant of a base shade for intensity control.
 *
 * Variants are derived from the base palette by simple transformations.
 * - Normal: base shade as defined in the palette.
 * - Bright: shade lightened toward white.
 * - Faint: slightly dimmed shade.
 */
enum class ColorVariant : std::uint8_t {
  Normal = 0, /*!< Base shade as-is */
  Bright,     /*!< Lightened variant */
  Faint,      /*!< Dimmed variant */
  Count       /*!< Number of variants (sentinel) */
};

/*!
 * \enum ColorLayer
 * \brief Target layer for color application.
 *
 * Foreground applies to text or glyphs, while Background applies to
 * the surrounding cell region in terminal or framebuffer rendering.
 */
enum class ColorLayer : std::uint8_t {
  Foreground = 0, /*!< Text or glyph foreground color */
  Background      /*!< Background cell color */
};

/*!
 * \struct ColorKey
 * \brief Compact descriptor of a color in GGEMS.
 *
 * A color key identifies a color by family, shade index, variant and
 * layer. It acts as a high-level handle used by UI code, which can be
 * converted into concrete RGB values or ANSI escape sequences.
 *
 * The struct is trivially comparable and suitable as a key in ordered
 * containers or for compile-time usage.
 */
struct ColorKey {
  ColorFamily family{};                        /*!< Color family identifier */
  std::uint8_t shade{};                         /*!< Shade index in [0, 11] */
  ColorVariant variant{ColorVariant::Normal}; /*!< Intensity variant */
  ColorLayer layer{ColorLayer::Foreground};   /*!< Target layer (FG/BG) */

  /*!
   * \brief Three-way comparison operator.
   *
   * Provides lexicographical comparison over \ref family, \ref shade,
   * \ref variant and \ref layer, enabling usage in ordered containers.
   * \return Comparison category determining ordering relationship.
   */
  constexpr auto operator<=>(ColorKey const &) const = default;
};

/*!
 * \brief Number of color families in the palette.
 */
inline constexpr std::size_t kColorFamilyCount =
    static_cast<std::size_t>(ColorFamily::Count);

/*!
 * \brief Number of available variants (Normal, Bright, Faint).
 */
inline constexpr std::size_t kColorVariantCount =
    static_cast<std::size_t>(ColorVariant::Count);

/*!
 * \brief Number of discrete shades per family.
 *
 * GGEMS currently defines 12 shades per color family.
 */
inline constexpr std::size_t kColorShadeCount = 13U;

/*!
 * \brief Palette type grouping shades by family.
 *
 * Outer index corresponds to \ref ggems::render::ColorFamily, inner index to
 * shade within that family.
 */
using ColorFamilyPalette =
    std::array<std::array<RGB, kColorShadeCount>, kColorFamilyCount>;

/*!
 * \brief Construct an \ref ggems::render::RGB triplet from channel values.
 *
 * \param r Red channel in [0, 255].
 * \param g Green channel in [0, 255].
 * \param b Blue channel in [0, 255].
 * \return RGB structure initialized with the provided channels.
 */
constexpr RGB MakeRGB(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
  return RGB{r, g, b};
}

/*!
 * \brief Build the grayscale family palette.
 *
 * The palette ranges from dark grays to near-white tones, providing
 * 12 discrete levels for neutral UI elements or backgrounds.
 *
 * \return Array of 12 grayscale RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeGrayScale() noexcept {
  return {
      MakeRGB(16, 16, 16),    MakeRGB(32, 32, 32),    MakeRGB(48, 48, 48),
      MakeRGB(64, 64, 64),    MakeRGB(96, 96, 96),    MakeRGB(128, 128, 128),
      MakeRGB(160, 160, 160), MakeRGB(192, 192, 192), MakeRGB(208, 208, 208),
      MakeRGB(224, 224, 224), MakeRGB(240, 240, 240), MakeRGB(252, 252, 252),
      MakeRGB(3, 3, 3)};
}

/*!
 * \brief Build the red family palette.
 *
 * Shades progress from deep reds to softer, warm tones suitable for
 * emphasis, warnings, or highlighting.
 *
 * \return Array of 12 red-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeRedScale() noexcept {
  return {MakeRGB(64, 0, 0),     MakeRGB(96, 0, 0),      MakeRGB(128, 0, 0),
          MakeRGB(160, 0, 0),    MakeRGB(192, 16, 16),   MakeRGB(220, 20, 60),
          MakeRGB(255, 40, 40),  MakeRGB(255, 80, 80),   MakeRGB(255, 99, 71),
          MakeRGB(255, 127, 80), MakeRGB(255, 160, 122), MakeRGB(255, 192, 160),
          MakeRGB(170, 45, 40)};
}

/*!
 * \brief Build the orange family palette.
 *
 * Includes dark copper-like tones up to bright amber/orange, used for
 * intermediate emphasis or category highlighting.
 *
 * \return Array of 12 orange-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeOrangeScale() noexcept {
  return {MakeRGB(80, 32, 0),    MakeRGB(96, 40, 0),    MakeRGB(128, 64, 0),
          MakeRGB(160, 80, 0),   MakeRGB(192, 96, 0),   MakeRGB(210, 105, 30),
          MakeRGB(255, 127, 80), MakeRGB(255, 140, 0),  MakeRGB(255, 165, 0),
          MakeRGB(255, 180, 40), MakeRGB(255, 200, 80), MakeRGB(255, 215, 120),
          MakeRGB(180, 88, 38)};
}

/*!
 * \brief Build the yellow family palette.
 *
 * Provides gold-like and bright yellow shades, suitable for warning
 * indicators, highlights, or energy-like visual cues.
 *
 * \return Array of 12 yellow-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeYellowScale() noexcept {
  return {
      MakeRGB(96, 96, 0),     MakeRGB(128, 128, 0),   MakeRGB(160, 144, 0),
      MakeRGB(192, 160, 0),   MakeRGB(210, 180, 0),   MakeRGB(238, 221, 130),
      MakeRGB(240, 230, 140), MakeRGB(250, 250, 120), MakeRGB(255, 255, 0),
      MakeRGB(255, 255, 80),  MakeRGB(255, 255, 160), MakeRGB(255, 255, 220),
      MakeRGB(190, 145, 45)};
}

/*!
 * \brief Build the green family palette.
 *
 * Shades range from deep greens to fresh, light tones for health,
 * activity, or success indicators.
 *
 * \return Array of 12 green-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeGreenScale() noexcept {
  return {MakeRGB(0, 48, 0),    MakeRGB(0, 80, 0),      MakeRGB(0, 100, 0),
          MakeRGB(0, 128, 0),   MakeRGB(0, 160, 64),    MakeRGB(0, 201, 87),
          MakeRGB(0, 255, 0),   MakeRGB(60, 255, 120),  MakeRGB(0, 255, 170),
          MakeRGB(46, 139, 87), MakeRGB(144, 238, 144), MakeRGB(204, 255, 204),
          MakeRGB(48, 220, 160)};
}

/*!
 * \brief Build the cyan family palette.
 *
 * Contains turquoise and light blue-greens, useful for informational
 * hints or secondary focus elements.
 *
 * \return Array of 12 cyan-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeCyanScale() noexcept {
  return {
      MakeRGB(0, 48, 48),     MakeRGB(0, 80, 80),     MakeRGB(0, 100, 100),
      MakeRGB(0, 128, 128),   MakeRGB(0, 160, 160),   MakeRGB(0, 183, 235),
      MakeRGB(0, 200, 255),   MakeRGB(0, 255, 255),   MakeRGB(80, 255, 255),
      MakeRGB(135, 206, 235), MakeRGB(180, 230, 255), MakeRGB(210, 245, 255),
      MakeRGB(58, 178, 190)};
}

/*!
 * \brief Build the blue family palette.
 *
 * Covers dark blues, azure tones, and light sky blues, suitable for
 * neutral or informational backgrounds and accents.
 *
 * \return Array of 12 blue-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeBlueScale() noexcept {
  return {MakeRGB(0, 0, 64),      MakeRGB(0, 0, 96),     MakeRGB(0, 0, 139),
          MakeRGB(25, 25, 112),   MakeRGB(0, 71, 171),   MakeRGB(30, 144, 255),
          MakeRGB(0, 127, 255),   MakeRGB(30, 38, 46),   MakeRGB(80, 120, 255),
          MakeRGB(125, 249, 255), MakeRGB(70, 130, 180), MakeRGB(160, 200, 255),
          MakeRGB(3, 5, 7)};
}

/*!
 * \brief Build the magenta family palette.
 *
 * Includes violet, pink, and purple-like shades, often used for
 * highlighting or categorical separation in the UI.
 *
 * \return Array of 12 magenta-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeMagentaScale() noexcept {
  return {
      MakeRGB(64, 0, 64),     MakeRGB(96, 0, 96),     MakeRGB(128, 0, 128),
      MakeRGB(139, 0, 139),   MakeRGB(186, 85, 211),  MakeRGB(199, 21, 133),
      MakeRGB(255, 0, 255),   MakeRGB(255, 30, 100),  MakeRGB(255, 105, 180),
      MakeRGB(238, 130, 238), MakeRGB(255, 160, 255), MakeRGB(255, 200, 240),
      MakeRGB(150, 62, 92)};
}

/*!
 * \brief Build the white/near-white family palette.
 *
 * Contains subtle variations around white for backgrounds and subtle
 * contrast, including warm and cold whites.
 *
 * \return Array of 12 white-family RGB shades.
 */
constexpr std::array<RGB, kColorShadeCount> MakeWhiteScale() noexcept {
  return {
      MakeRGB(255, 255, 255), MakeRGB(250, 250, 250), MakeRGB(245, 245, 245),
      MakeRGB(240, 240, 235), MakeRGB(235, 232, 220), MakeRGB(230, 230, 230),
      MakeRGB(220, 220, 220), MakeRGB(210, 210, 210), MakeRGB(200, 200, 200),
      MakeRGB(185, 185, 185), MakeRGB(255, 255, 240), MakeRGB(255, 255, 200),
      MakeRGB(218, 214, 196)};
}

/*!
 * \brief Build the base palette for all families in the Normal variant.
 *
 * \return Complete \ref ggems::render::ColorFamilyPalette for Normal variant.
 */
constexpr ColorFamilyPalette MakeBasePalette() noexcept {
  return ColorFamilyPalette{
      MakeGrayScale(),   MakeRedScale(),     MakeOrangeScale(),
      MakeYellowScale(), MakeGreenScale(),   MakeCyanScale(),
      MakeBlueScale(),   MakeMagentaScale(), MakeWhiteScale()};
}

/*!
 * \brief Base "Normal" variant palette for all color families.
 */
inline constexpr ColorFamilyPalette kBasePalette = MakeBasePalette();

/*!
 * \brief Brighten a single color channel.
 *
 * The transformation interpolates toward white as:
 * \f$ c \gets c + (255 - c) / 3 \f$.
 *
 * \param c Input channel value in [0,255].
 * \return Brightened channel value in [0,255].
 */
constexpr std::uint8_t BrightenChannel(std::uint8_t c) noexcept {
  return static_cast<std::uint8_t>(c + (255U - c) / 3U);
}

/*!
 * \brief Dim a single color channel.
 *
 * The transformation scales the channel down to two thirds of its
 * intensity:
 * \f$ c \gets \lfloor 2c / 3 \rfloor \f$.
 *
 * \param c Input channel value in [0,255].
 * \return Dimmed channel value in [0,255].
 */
constexpr std::uint8_t FaintChannel(std::uint8_t c) noexcept {
  return static_cast<std::uint8_t>((static_cast<std::uint16_t>(c) * 2U) / 3U);
}

/*!
 * \brief Apply a \ref ggems::render::ColorVariant transformation to a base
 * color.
 *
 * \param base Base RGB color.
 * \param v Variant to apply (Normal, Bright, Faint).
 * \return Transformed color according to \p v.
 */
constexpr RGB ApplyVariant(RGB base, ColorVariant v) noexcept {
  switch (v) {
  case ColorVariant::Normal:
    return base;
  case ColorVariant::Bright:
    return RGB{BrightenChannel(base.r), BrightenChannel(base.g),
               BrightenChannel(base.b)};
  case ColorVariant::Faint:
    return RGB{FaintChannel(base.r), FaintChannel(base.g),
               FaintChannel(base.b)};
  default:
    return base;
  }
}

/*!
 * \brief Retrieve an RGB color from the base palette and variant.
 *
 * \param family Color family index.
 * \param shade Shade index in [0, 11]. Values above the maximum are
 *              clamped to the last valid shade.
 * \param variant Intensity variant (Normal, Bright, Faint).
 * \return Resulting RGB color.
 */
constexpr RGB GetColorRGB(ColorFamily family, std::uint8_t shade,
                           ColorVariant variant) noexcept {
  const auto family_index = static_cast<std::size_t>(family);
  const auto shade_index = static_cast<std::size_t>(
      std::min<std::uint8_t>(shade, kColorShadeCount - 1U));

  const RGB base = kBasePalette[family_index][shade_index];
  return ApplyVariant(base, variant);
}

/*!
 * \enum AnsiControl
 * \brief Basic ANSI control codes for text attributes.
 *
 * These values abstract simple SGR control sequences such as resetting
 * attributes, restoring colors, enabling bold or faint text.
 */
enum class AnsiControl : std::uint8_t {
  ResetAll,    /*!< Reset all attributes and colors */
  ResetColor, /*!< Restore default foreground/background colors */
  Bold,        /*!< Enable bold/intense rendering */
  Faint        /*!< Enable faint/dim rendering */
};

/*!
 * \brief Return the raw ANSI escape sequence for a control code.
 *
 * \param c Control code.
 * \return Null-terminated escape sequence as a string view.
 */
inline std::string_view AnsiControlCode(AnsiControl c) noexcept {
  switch (c) {
  case AnsiControl::ResetAll:
    return "\033[0m";
  case AnsiControl::ResetColor:
    return "\033[39;49m";
  case AnsiControl::Bold:
    return "\033[1m";
  case AnsiControl::Faint:
    return "\033[2m";
  default:
    return "\033[0m";
  }
}

/*!
 * \brief Append an ANSI 24-bit color sequence to a string.
 *
 * \param out Output string to append to.
 * \param key Color key describing family, shade, variant, and layer.
 *
 * The function computes the RGB value from the palette and emits an ANSI
 * true-color escape sequence for either the foreground or background
 * depending on \ref ggems::render::ColorLayer.
 */
inline void AppendAnsiColor(std::string &out, ColorKey const &key) {
  const RGB rgb = GetColorRGB(key.family, key.shade, key.variant);

  const int code = (key.layer == ColorLayer::Foreground) ? 38 : 48;

  std::format_to(std::back_inserter(out), "\033[{};2;{};{};{}m", code,
                 static_cast<unsigned int>(rgb.r),
                 static_cast<unsigned int>(rgb.g),
                 static_cast<unsigned int>(rgb.b));
}

/*!
 * \brief Build an ANSI 24-bit color sequence as a standalone string.
 *
 * \param key Color key describing family, shade, variant, and layer.
 * \return Newly allocated string containing the ANSI escape sequence.
 */
inline std::string AnsiColor(ColorKey const &key) {
  const RGB rgb = GetColorRGB(key.family, key.shade, key.variant);

  const int code = (key.layer == ColorLayer::Foreground) ? 38 : 48;

  // Retour direct : une seule allocation très légère
  return std::format("\033[{};2;{};{};{}m", code, static_cast<unsigned>(rgb.r),
                     static_cast<unsigned>(rgb.g),
                     static_cast<unsigned>(rgb.b));
}

/*!
 * \brief Append an ANSI control sequence to a string.
 *
 * \param out Output string to append to.
 * \param c Control code to append.
 */
inline void AppendAnsiControl(std::string &out, AnsiControl c) {
  out.append(AnsiControlCode(c));
}

/*!
 * \brief Return the ANSI control sequence for a given code.
 *
 * This helper simply forwards to \ref ggems::render::AnsiControlCode and is
 * provided for symmetry with \ref ggems::render::AnsiColor.
 *
 * \param c Control code.
 * \return Null-terminated escape sequence as a string view.
 */
inline std::string_view AnsiControl(AnsiControl c) {
  switch (c) {
  case AnsiControl::ResetAll:
    return "\033[0m";
  case AnsiControl::ResetColor:
    return "\033[39;49m";
  case AnsiControl::Bold:
    return "\033[1m";
  case AnsiControl::Faint:
    return "\033[2m";
  default:
    return "\033[0m";
  }
}

/*!
 * \brief Construct a \ref ggems::render::ColorKey from its components.
 *
 * \param family Color family.
 * \param shade Shade index in [0, 11].
 * \param variant Intensity variant (Normal, Bright, Faint).
 * \param layer Target layer (Foreground or Background).
 * \return ColorKey describing the requested color.
 */
constexpr ColorKey
MakeColor(ColorFamily family, std::uint8_t shade,
           ColorVariant variant = ColorVariant::Normal,
           ColorLayer layer = ColorLayer::Foreground) noexcept {
  return ColorKey{family, shade, variant, layer};
}
} // namespace ggems::render
