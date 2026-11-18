// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Imports the bounded MHD/RAW profile and exact local material palette.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <ios>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace ggems::geometry {

namespace {

/*!
 * \brief Removes surrounding ASCII whitespace from one imported field.
 * \param[in] text Borrowed input field.
 * \return Trimmed view into the same input.
 */
auto Trim(std::string_view text) noexcept -> std::string_view {
  auto const first = text.find_first_not_of(" \t\r\n\f\v");

  if (first == std::string_view::npos) {
    return {};
  }

  return text.substr(first, text.find_last_not_of(" \t\r\n\f\v") - first + 1U);
}

/*!
 * \brief Parses the exact numeric arity of one admitted metadata field.
 * \tparam Number Integer or floating-point field type.
 * \tparam Count Required number of values.
 * \param[in] text Whitespace-separated numeric values.
 * \param[in] file Diagnostic input path.
 * \param[in] field Diagnostic field name.
 * \return Parsed values without trailing tokens.
 * \throws core::GGEMSRecoverable If a value or the field arity is invalid.
 */
template <typename Number, std::size_t Count>
auto ParseValues(std::string_view text, std::filesystem::path const &file,
                 std::string_view field) -> std::array<Number, Count> {
  std::array<Number, Count> values{};

  for (auto &value : values) {
    text = Trim(text);

    auto const separator = text.find_first_of(" \t\r\n\f\v");
    auto const token = text.substr(0U, separator);

    if (token.empty()) {
      throw core::GGEMSRecoverable(
        std::format("{}: missing value in {}.", file.string(), field));
    }

    auto const parsed =
      std::from_chars(token.data(), token.data() + token.size(), value);

    if (parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size()) {
      throw core::GGEMSRecoverable(
        std::format("{}: invalid {} value '{}'.", file.string(), field, token));
    }

    text = separator == std::string_view::npos ? std::string_view{}
                                               : text.substr(separator);
  }

  if (!Trim(text).empty()) {
    throw core::GGEMSRecoverable(
      std::format("{}: extra values in {}.", file.string(), field));
  }

  return values;
}

/*!
 * \brief Reads the absent or complete two-column presentation suffix.
 * \param[in,out] row Mapping row positioned after MIN MAX MATERIAL.
 * \param[in] default_color Exact named Material's presentation color.
 * \param[in] material_file Mapping path for input diagnostics.
 * \param[in] line_number One-based mapping line.
 * \return Solid with the Material color, or the explicit mode and color.
 * \throws core::GGEMSRecoverable If suffix arity, mode, or color is invalid.
 */
auto ReadDisplayStyle(std::istringstream &row, render::ColorKey default_color,
                      std::filesystem::path const &material_file,
                      std::size_t line_number) -> GGEMSVoxelDisplayStyle {
  GGEMSVoxelDisplayStyle style{
    .mode = render::GGEMSGeometryDisplayMode::Solid,
    .color = default_color,
  };

  std::string mode;
  if (!(row >> mode)) {
    return style;
  }

  std::string color;
  std::string extra;

  if (!(row >> color) || (row >> extra)) {
    throw core::GGEMSRecoverable(std::format(
      "{}: line {}: expected MIN MAX MATERIAL or MIN MAX MATERIAL MODE COLOR.",
      material_file.string(), line_number));
  }

  if (mode == "None") {
    style.mode = render::GGEMSGeometryDisplayMode::None;
  } else if (mode == "Wireframe") {
    style.mode = render::GGEMSGeometryDisplayMode::Wireframe;
  } else if (mode != "Solid") {
    throw core::GGEMSRecoverable(
      std::format("{}: line {}: unknown display mode '{}'; expected None, "
                  "Wireframe, or Solid.",
                  material_file.string(), line_number, mode));
  }

  auto const resolved_color = render::ParseColorName(color);

  if (!resolved_color) {
    throw core::GGEMSRecoverable(
      std::format("{}: line {}: unknown GGEMS color '{}'.",
                  material_file.string(), line_number, color));
  }

  style.color = *resolved_color;

  return style;
}

} // namespace

/*! \brief Shared immutable import data, separate from the authored pose. */
struct GGEMSVoxelizedVolume::Data {
  /*!
   * \brief Validates MHD metadata and initializes the centered grid dimensions.
   * \param[in] image_file MHD metadata path.
   * \return External RAW path resolved against the MHD directory.
   * \throws core::GGEMSRecoverable If metadata or canonical quantities are
   * invalid.
   */
  auto ReadMetadata(std::filesystem::path const &image_file)
    -> std::filesystem::path;

  /*!
   * \brief Loads the exact original label field after metadata admission.
   * \param[in] raw_path Resolved external RAW path.
   * \throws core::GGEMSRecoverable If the file size or read fails.
   */
  auto ReadLabels(std::filesystem::path const &raw_path) -> void;

  /*!
   * \brief Builds physical palette labels and separate mapping presentation.
   * \param[in] material_file Three- or five-column label mapping path.
   * \throws core::GGEMSRecoverable If ranges, materials, display styles, or
   * label coverage fail.
   */
  auto BuildMaterialLabels(std::filesystem::path const &material_file) -> void;

  /*! \brief Voxel counts in X, Y, Z order. */
  std::array<std::size_t, 3> dimensions{};

  /*! \brief Canonical picometer spacing in X, Y, Z order. */
  std::array<units::Length, 3> spacing{};

  /*! \brief Full local sizes, including any odd picometer value. */
  std::array<units::Length, 3> size{};

  /*! \brief Original labels with X varying fastest. */
  std::vector<std::uint16_t> labels;

  /*! \brief Owned values deduplicated by exact scientific identity. */
  std::vector<core::materials::GGEMSMaterial> materials;

  /*! \brief One compact local material-palette index per voxel. */
  std::vector<std::uint16_t> material_labels;

  /*! \brief One style per mapping row, sorted by exclusive label endpoint. */
  std::vector<GGEMSVoxelDisplayMapping> display_styles;
};

// =============================================================================
// =============================================================================

auto GGEMSVoxelizedVolume::Data::ReadMetadata(
  std::filesystem::path const &image_file) -> std::filesystem::path {
  std::ifstream metadata{image_file};

  if (!metadata) {
    throw core::GGEMSRecoverable(
      std::format("Cannot read MHD file '{}'.", image_file.string()));
  }

  std::array<std::string_view, 14> const admitted_fields{
    "ObjectType",
    "NDims",
    "BinaryData",
    "BinaryDataByteOrderMSB",
    "CompressedData",
    "DimSize",
    "ElementSpacing",
    "ElementType",
    "ElementDataFile",
    "TransformMatrix",
    "Offset",
    "Position",
    "ElementNumberOfChannels",
    "HeaderSize",
  };

  std::map<std::string, std::string, std::less<>> fields;
  std::string line;
  std::size_t line_number{};

  while (std::getline(metadata, line)) {
    ++line_number;

    auto const text = Trim(line);

    if (text.empty() || text.starts_with('#')) {
      continue;
    }

    auto const separator = text.find('=');

    if (separator == std::string_view::npos) {
      throw core::GGEMSRecoverable(
        std::format("{}: line {}: expected MHD key = value.",
                    image_file.string(), line_number));
    }

    auto const key = Trim(text.substr(0U, separator));
    auto const value = Trim(text.substr(separator + 1U));

    if (std::ranges::find(admitted_fields, key) == admitted_fields.end()) {
      throw core::GGEMSRecoverable(std::format(
        "{}: unsupported MHD field '{}'.", image_file.string(), key));
    }

    if (value.empty() ||
        !fields.emplace(std::string{key}, std::string{value}).second) {
      throw core::GGEMSRecoverable(std::format(
        "{}: empty or repeated MHD field '{}'.", image_file.string(), key));
    }
  }

  if (metadata.bad()) {
    throw core::GGEMSRecoverable(
      std::format("Failed reading MHD file '{}'.", image_file.string()));
  }

  auto required = [&](std::string_view key) -> std::string const & {
    auto const found = fields.find(key);

    if (found == fields.end()) {
      throw core::GGEMSRecoverable(
        std::format("{}: missing MHD field '{}'.", image_file.string(), key));
    }

    return found->second;
  };

  std::array<std::pair<std::string_view, std::string_view>, 6> const profile{
    {
      {"ObjectType", "Image"},
      {"NDims", "3"},
      {"BinaryData", "True"},
      {"BinaryDataByteOrderMSB", "False"},
      {"CompressedData", "False"},
      {"ElementType", "MET_USHORT"},
    },
  };

  for (auto const &[key, value] : profile) {
    if (required(key) != value) {
      throw core::GGEMSRecoverable(std::format(
        "{}: MHD {} must be '{}'.", image_file.string(), key, value));
    }
  }

  if (auto const found = fields.find("TransformMatrix");
      found != fields.end()) {
    auto const matrix =
      ParseValues<double, 9>(found->second, image_file, "TransformMatrix");

    std::array<double, 9> const identity{1, 0, 0, 0, 1, 0, 0, 0, 1};

    if (matrix != identity) {
      throw core::GGEMSRecoverable(
        "VoxelizedVolume only admits the identity MHD TransformMatrix.");
    }
  }

  for (auto const *const key : {"Offset", "Position"}) {
    if (auto const found = fields.find(key); found != fields.end()) {
      if (ParseValues<double, 3>(found->second, image_file, key) !=
          std::array<double, 3>{}) {
        throw core::GGEMSRecoverable(
          std::format("VoxelizedVolume only admits zero MHD {}.", key));
      }
    }
  }

  for (auto const &[key, value] :
       std::array<std::pair<std::string_view, std::uint32_t>, 2>{
         {{"ElementNumberOfChannels", 1U}, {"HeaderSize", 0U}},
       }) {
    if (auto const found = fields.find(key); found != fields.end()) {
      if (ParseValues<std::uint32_t, 1>(found->second, image_file, key)[0] !=
          value) {
        throw core::GGEMSRecoverable(
          std::format("VoxelizedVolume requires MHD {} = {}.", key, value));
      }
    }
  }

  dimensions =
    ParseValues<std::size_t, 3>(required("DimSize"), image_file, "DimSize");

  auto const spacing_mm = ParseValues<long double, 3>(
    required("ElementSpacing"), image_file, "ElementSpacing");

  for (std::size_t axis = 0U; axis < dimensions.size(); ++axis) {
    auto const count = dimensions[axis];
    auto const converted =
      units::MakeQuantity<units::Length>(spacing_mm[axis], "mm");

    if (count == 0U || !converted || converted->value == 0U) {
      throw core::GGEMSRecoverable(
        "Voxel dimensions and spacing must be positive and representable.");
    }

    spacing[axis] = *converted;
    size[axis] = units::Length{converted->value * count};
  }

  std::filesystem::path const raw_name{required("ElementDataFile")};

  auto const raw_text = raw_name.string();
  if (raw_name.has_root_path() || raw_text == "LOCAL" || raw_text == "LIST" ||
      raw_text.starts_with("LIST ") ||
      raw_text.find_first_of("\t\r\n%*?") != std::string::npos) {
    throw core::GGEMSRecoverable(
      "ElementDataFile must name one relative external RAW file.");
  }

  return image_file.parent_path() / raw_name;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::Data::ReadLabels(
  std::filesystem::path const &raw_path) -> void {
  std::ifstream raw{raw_path, std::ios::binary | std::ios::ate};

  if (!raw) {
    throw core::GGEMSRecoverable(
      std::format("Cannot read RAW file '{}'.", raw_path.string()));
  }

  auto const voxel_count = dimensions[0] * dimensions[1] * dimensions[2];
  auto const byte_count = voxel_count * sizeof(std::uint16_t);
  auto const file_size = static_cast<std::streamoff>(raw.tellg());

  if (!std::cmp_equal(file_size, byte_count)) {
    throw core::GGEMSRecoverable(std::format(
      "{}: RAW size must be exactly {} bytes.", raw_path.string(), byte_count));
  }

  labels.resize(voxel_count);
  raw.seekg(0, std::ios::beg);
  raw.read(reinterpret_cast<char *>(labels.data()),
           static_cast<std::streamsize>(byte_count));

  if (!raw) {
    throw core::GGEMSRecoverable(
      std::format("Failed reading RAW file '{}'.", raw_path.string()));
  }

  if constexpr (std::endian::native == std::endian::big) {
    for (auto &label : labels) {
      label = std::byteswap(label);
    }
  }
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::Data::BuildMaterialLabels(
  std::filesystem::path const &material_file) -> void {
  std::ifstream mapping{material_file};

  if (!mapping) {
    throw core::GGEMSRecoverable(
      std::format("Cannot read material file '{}'.", material_file.string()));
  }

  auto constexpr label_count =
    static_cast<std::uint32_t>(std::numeric_limits<std::uint16_t>::max()) + 1U;

  auto constexpr unmapped = std::numeric_limits<std::uint32_t>::max();

  std::vector<std::uint32_t> material_indices(label_count, unmapped);

  auto &manager = core::materials::GGEMSMaterialManager::GetInstance();

  std::size_t line_number{};
  std::string line;
  while (std::getline(mapping, line)) {
    ++line_number;
    auto const text = Trim(line);
    if (text.empty() || text.starts_with('#')) {
      continue;
    }

    std::istringstream row{std::string{text}};
    std::string minimum;
    std::string maximum;
    std::string name;

    if (!(row >> minimum >> maximum >> name)) {
      throw core::GGEMSRecoverable(
        std::format("{}: line {}: expected MIN MAX MATERIAL or "
                    "MIN MAX MATERIAL MODE COLOR.",
                    material_file.string(), line_number));
    }

    auto const first =
      ParseValues<std::uint32_t, 1>(minimum, material_file, "MIN")[0];

    auto const last =
      ParseValues<std::uint32_t, 1>(maximum, material_file, "MAX")[0];

    if (first >= label_count || last > label_count || last < first) {
      throw core::GGEMSRecoverable(
        std::format("{}: line {}: invalid uint16 label range.",
                    material_file.string(), line_number));
    }

    auto const end = first == last ? first + 1U : last;
    for (auto label = first; label < end; ++label) {
      if (material_indices[label] != unmapped) {
        throw core::GGEMSRecoverable(
          std::format("{}: line {}: overlapping label {}.",
                      material_file.string(), line_number, label));
      }
    }

    auto const &material = manager.Require(manager.GetOrAdd(name));
    auto const style =
      ReadDisplayStyle(row, material.GetColor(), material_file, line_number);

    auto const existing =
      std::ranges::find_if(materials, [&](auto const &candidate) -> bool {
        return core::materials::HasSameScientificIdentity(candidate, material);
      });

    auto const local_index =
      static_cast<std::uint32_t>(existing - materials.begin());

    if (existing == materials.end()) {
      materials.push_back(material);
    }

    for (auto label = first; label < end; ++label) {
      material_indices[label] = local_index;
    }

    display_styles.push_back({
      .first_label = first,
      .end_label = end,
      .material_name = std::move(name),
      .style = style,
    });
  }

  if (mapping.bad()) {
    throw core::GGEMSRecoverable(std::format(
      "Failed reading material file '{}'.", material_file.string()));
  }

  std::ranges::sort(display_styles, {}, &GGEMSVoxelDisplayMapping::end_label);
  material_labels.reserve(labels.size());

  for (auto const label : labels) {
    auto const material_index = material_indices[label];
    if (material_index == unmapped) {
      throw core::GGEMSRecoverable(
        std::format("{}: RAW label {} has no material mapping.",
                    material_file.string(), label));
    }
    material_labels.push_back(static_cast<std::uint16_t>(material_index));
  }
}

// =============================================================================
// =============================================================================

GGEMSVoxelizedVolume::GGEMSVoxelizedVolume(
  std::filesystem::path const &image_file,
  std::filesystem::path const &material_file)
    : GGEMSVoxelizedVolume(std::string{}, image_file, material_file) {}

// -----------------------------------------------------------------------------

GGEMSVoxelizedVolume::GGEMSVoxelizedVolume(
  std::string name, std::filesystem::path const &image_file,
  std::filesystem::path const &material_file)
    : name_(std::move(name)) {
  auto data = std::make_shared<Data>();
  auto const raw_path = data->ReadMetadata(image_file);
  data->ReadLabels(raw_path);
  data->BuildMaterialLabels(material_file);

  data_ = std::move(data);
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::SetPosition(Position3PM center) noexcept
  -> GGEMSVoxelizedVolume & {
  center_pm_ = center;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::SetRotation(units::Angle angle_x,
                                       units::Angle angle_y,
                                       units::Angle angle_z)
  -> GGEMSVoxelizedVolume & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::Verbose() -> GGEMSVoxelizedVolume & {
  auto report =
    std::format("Voxelized volume: {}\n"
                "  Dimensions (X, Y, Z): {} x {} x {}\n"
                "  Spacing (X, Y, Z): {}, {}, {}\n"
                "  Full size (X, Y, Z): {}, {}, {}\n"
                "  Voxel count: {}\n"
                "  World center (X, Y, Z): {}, {}, {}\n"
                "  Local axes in World coordinates:\n"
                "    X: ({:.6g}, {:.6g}, {:.6g})\n"
                "    Y: ({:.6g}, {:.6g}, {:.6g})\n"
                "    Z: ({:.6g}, {:.6g}, {:.6g})\n"
                "  Original label mappings (half-open ranges):\n",
                name_.empty() ? "(unnamed)" : name_, data_->dimensions[0],
                data_->dimensions[1], data_->dimensions[2],
                units::HumanReadable(data_->spacing[0], 3),
                units::HumanReadable(data_->spacing[1], 3),
                units::HumanReadable(data_->spacing[2], 3),
                units::HumanReadable(data_->size[0], 3),
                units::HumanReadable(data_->size[1], 3),
                units::HumanReadable(data_->size[2], 3), data_->labels.size(),
                units::HumanReadableSignedLength(center_pm_.x, 3),
                units::HumanReadableSignedLength(center_pm_.y, 3),
                units::HumanReadableSignedLength(center_pm_.z, 3),
                rotation_.axis_x.x, rotation_.axis_x.y, rotation_.axis_x.z,
                rotation_.axis_y.x, rotation_.axis_y.y, rotation_.axis_y.z,
                rotation_.axis_z.x, rotation_.axis_z.y, rotation_.axis_z.z);

  constexpr std::array mode_names{"None", "Wireframe", "Solid"};
  for (auto const &mapping : data_->display_styles) {
    report +=
      std::format("    [{}, {}): {}; mode={}; color={}\n", mapping.first_label,
                  mapping.end_label, mapping.material_name,
                  mode_names[static_cast<std::size_t>(mapping.style.mode)],
                  render::GetColorName(mapping.style.color));
  }

  report += "  Local Material palette:\n";

  for (std::size_t index = 0U; index < data_->materials.size(); ++index) {
    auto const &material = data_->materials[index];
    report +=
      std::format("    [{}] {}; density={}\n", index, material.GetName(),
                  units::HumanReadable(material.GetDensity(), 6));
  }

  GGEMS_INFO("Geometry", "{}", report);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetRotation() const noexcept
  -> GGEMSRotation3D const & {
  return rotation_;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetColor() const noexcept -> render::ColorKey {
  return render::WHITE_White;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetDimensions() const noexcept
  -> std::array<std::size_t, 3> const & {
  return data_->dimensions;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetSpacing() const noexcept
  -> std::array<units::Length, 3> const & {
  return data_->spacing;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetSize() const noexcept
  -> std::array<units::Length, 3> const & {
  return data_->size;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetLabels() const noexcept
  -> std::span<std::uint16_t const> {
  return data_->labels;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetMaterials() const noexcept
  -> std::span<core::materials::GGEMSMaterial const> {
  return data_->materials;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetMaterialLabels() const noexcept
  -> std::span<std::uint16_t const> {
  return data_->material_labels;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetDisplayStyle(std::uint16_t original_label)
  const noexcept -> GGEMSVoxelDisplayStyle const & {
  return data_->display_styles[GetDisplayMappingIndex(original_label)].style;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetDisplayMappings() const noexcept
  -> std::span<GGEMSVoxelDisplayMapping const> {
  return data_->display_styles;
}

// -----------------------------------------------------------------------------

auto GGEMSVoxelizedVolume::GetDisplayMappingIndex(
  std::uint16_t original_label) const noexcept -> std::uint32_t {
  auto const range =
    std::ranges::upper_bound(data_->display_styles, original_label, {},
                             &GGEMSVoxelDisplayMapping::end_label);

  return static_cast<std::uint32_t>(range - data_->display_styles.begin());
}

} // namespace ggems::geometry
