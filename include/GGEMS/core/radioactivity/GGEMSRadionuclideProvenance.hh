#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace ggems::core::radioactivity {

class GGEMSRadionuclideProvenance {
public:
  GGEMSRadionuclideProvenance(
      std::string authority, std::string citation,
      std::string evaluation_date_or_version,
      std::optional<std::string> reference_identifier = std::nullopt);

  [[nodiscard]] auto GetAuthority() const noexcept -> std::string_view {
    return authority_;
  }

  [[nodiscard]] auto GetCitation() const noexcept -> std::string_view {
    return citation_;
  }

  [[nodiscard]] auto GetEvaluationDateOrVersion() const noexcept
      -> std::string_view {
    return evaluation_date_or_version_;
  }

  [[nodiscard]] auto GetReferenceIdentifier() const noexcept
      -> std::optional<std::string> const & {
    return reference_identifier_;
  }

private:
  std::string authority_;
  std::string citation_;
  std::string evaluation_date_or_version_;
  std::optional<std::string> reference_identifier_;
};

} // namespace ggems::core::radioactivity
