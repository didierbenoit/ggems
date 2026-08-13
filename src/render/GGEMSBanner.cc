#include <array>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <cstddef>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"

namespace {

// =============================================================================
// =============================================================================

constexpr std::size_t number_lines = 17U;
using BannerSnapshot = std::array<std::u32string_view, number_lines>;

// =============================================================================
// =============================================================================

constexpr BannerSnapshot ascii_banner{{
    U"+****************************************************+",
    U"*                                                    *",
    U"*    ######   ######  ####### ###    ### #######     *",
    U"*   ##       ##       ##      ####  #### ##          *",
    U"*   ##   ### ##   ### #####   ## #### ## #######     *",
    U"*   ##    ## ##    ## ##      ##  ##  ##      ##     *",
    U"*    ######   ######  ####### ##      ## #######     *",
    U"*   ******** ******** ******* ********** *******     *",
    U"*                                                    *",
    U"+----------------------------------------------------+",
    U"*                                                    *",
    U"*      GPU Geant4-based Monte Carlo Simulations      *",
    U"*    Version 2.0 . GGEMS Team . https://ggems.fr     *",
    U"*       Authors: Julien Bert & Didier Benoit         *",
    U"*   Copyright (C) 2026 Licensed under GNU GPL v3.0   *",
    U"*                                                    *",
    U"+****************************************************+",
}};

// =============================================================================
// =============================================================================

constexpr BannerSnapshot unicode_banner{{
    U"╔════════════════════════════════════════════════════╗",
    U"║                                                    ║",
    U"║    ██████╗  ██████╗ ███████╗███╗   ███╗███████╗    ║",
    U"║   ██╔════╝ ██╔════╝ ██╔════╝████╗ ████║██╔════╝    ║",
    U"║   ██║  ███╗██║  ███╗█████╗  ██╔████╔██║███████╗    ║",
    U"║   ██║   ██║██║   ██║██╔══╝  ██║╚██╔╝██║╚════██║    ║",
    U"║   ╚██████╔╝╚██████╔╝███████╗██║ ╚═╝ ██║███████║    ║",
    U"║    ╚═════╝  ╚═════╝ ╚══════╝╚═╝     ╚═╝╚══════╝    ║",
    U"║                                                    ║",
    U"╟────────────────────────────────────────────────────╢",
    U"║                                                    ║",
    U"║      GPU Geant4-based Monte Carlo Simulations      ║",
    U"║    Version 2.0 • GGEMS Team • https://ggems.fr     ║",
    U"║       Authors: Julien Bert & Didier Benoit         ║",
    U"║   Copyright (C) 2026 Licensed under GNU GPL v3.0   ║",
    U"║                                                    ║",
    U"╚════════════════════════════════════════════════════╝",
}};

// =============================================================================
// =============================================================================

constexpr ggems::render::ColorKey banner_color = ggems::render::GREEN_Acid;

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetBannerSnapshot() noexcept -> BannerSnapshot const & {
  if (ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
      ggems::core::Encoding::Ascii) {
    return ascii_banner;
  }

  return unicode_banner;
}

} // namespace

namespace ggems::render {

// =============================================================================
// =============================================================================

auto BuildBannerLines() -> std::vector<WrappedLine> {
  auto const &snapshot = GetBannerSnapshot();

  std::vector<WrappedLine> lines;
  lines.reserve(snapshot.size());

  for (auto const text : snapshot) {
    WrappedLine line;
    line.segments.push_back(
        {.text = std::u32string{text}, .color = banner_color});
    lines.push_back(std::move(line));
  }

  return lines;
}

} // namespace ggems::render
