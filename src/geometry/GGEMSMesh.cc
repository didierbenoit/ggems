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
 * \brief Imports triangulated OBJ and STL data without physical admission.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>
#include <span>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSMesh.hh"
#include "GGEMS/geometry/GGEMSRigidPose.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace ggems::geometry {
namespace {

/*! \brief Identifies the external record for an import failure diagnostic. */
struct InputLocation {
  /*! \brief One-based OBJ line, or zero when reading STL. */
  std::size_t line{};

  /*! \brief One-based STL facet, or zero when outside facet parsing. */
  std::size_t facet{};
};

/*!
 * \brief Reports a source-boundary failure with the imported path.
 * \param[in] file Source filename supplied by the caller.
 * \param[in] reason Specific violated import requirement.
 * \param[in] location Source record when available.
 * \throws GGEMSRecoverable Always, retaining filename and record context.
 */
[[noreturn]] auto InvalidMesh(std::filesystem::path const &file,
                              std::string_view reason,
                              InputLocation location = {}) -> void {
  std::string context;

  if (location.line != 0) {
    context = std::format(" at line {}", location.line);
  } else if (location.facet != 0) {
    context = std::format(" at facet {}", location.facet);
  }

  throw core::GGEMSRecoverable(
    std::format("Mesh '{}'{}: {}", file.string(), context, reason));
}

/*!
 * \brief Removes surrounding OBJ/STL ASCII whitespace.
 * \param[in] text Borrowed source text.
 * \return Trimmed view into the same storage, or an empty view.
 */
auto Trim(std::string_view text) -> std::string_view {
  auto const first = text.find_first_not_of(" \t\r\n");

  if (first == std::string_view::npos) {
    return {};
  }

  auto const last = text.find_last_not_of(" \t\r\n");

  return text.substr(first, last - first + 1U);
}

/*!
 * \brief Consumes one whitespace-delimited token without allocating.
 * \param[in,out] text Remaining source fields, advanced past the token.
 * \return Token borrowing the original source storage.
 */
auto TakeToken(std::string_view &text) -> std::string_view {
  text = Trim(text);

  auto const end = text.find_first_of(" \t\r\n");
  auto const token = text.substr(0, end);

  text = end == std::string_view::npos ? std::string_view{} : text.substr(end);

  return token;
}

/*!
 * \brief Parses one complete locale-independent source number.
 * \tparam Number Source coordinate or signed OBJ index representation.
 * \param[in] text Complete numeric token, allowing a single leading plus.
 * \param[in] file Source filename for boundary diagnostics.
 * \param[in] location Source record being parsed.
 * \return Parsed value; coordinate finiteness is checked at conversion.
 */
template <typename Number>
auto ReadNumber(std::string_view text, std::filesystem::path const &file,
                InputLocation location = {}) -> Number {
  if (text.starts_with('+')) {
    text.remove_prefix(1);
    if (text.starts_with('-')) {
      InvalidMesh(file, "invalid numeric sign", location);
    }
  }

  if (text.empty()) {
    InvalidMesh(file, "missing numeric field", location);
  }

  Number value{};
  auto const [end, error] =
    std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size()) {
    InvalidMesh(file, "invalid numeric field", location);
  }

  return value;
}

/*!
 * \brief Expands local bounds with one referenced FP32 vertex.
 * \param[in,out] bounds Initialized minimum and maximum in local pm.
 * \param[in] vertex Referenced local position in pm.
 */
auto ExtendBounds(GGEMSMeshBounds &bounds, std::array<float, 3> const &vertex)
  -> void {
  for (std::size_t axis = 0; axis < vertex.size(); ++axis) {
    bounds.minimum[axis] = std::min(bounds.minimum[axis], vertex[axis]);
    bounds.maximum[axis] = std::max(bounds.maximum[axis], vertex[axis]);
  }
}

/*!
 * \brief Converts a source position through the central Units authority.
 * \param[in] source Finite coordinates in the declared source unit.
 * \param[in] data Import unit and filename metadata.
 * \param[in] location Source record for conversion failures.
 * \return Local FP32 picometers, without integral-picometer rounding.
 */
auto ConvertVertex(std::array<double, 3> const &source,
                   GGEMSMeshData const &data, InputLocation location)
  -> std::array<float, 3> {
  using MeshCoordinate = units::Quantity<units::PositionCoordinateTag, float>;

  std::array<float, 3> vertex{};
  for (std::size_t axis = 0; axis < vertex.size(); ++axis) {
    auto const converted =
      units::MakeQuantity<MeshCoordinate>(source[axis], data.source_unit);

    if (!converted) {
      InvalidMesh(data.source_file,
                  "coordinates must be finite and representable in FP32 pm",
                  location);
    }

    vertex[axis] = converted->value;
  }

  return vertex;
}

/*!
 * \brief Appends one representable position within the compact index limit.
 * \param[in,out] data Accumulated imported positions and source metadata.
 * \param[in] source Coordinates in the declared source unit.
 * \param[in] location Source record for conversion or capacity failures.
 * \return Global compact index of the appended position.
 */
auto AppendVertex(GGEMSMeshData &data, std::array<double, 3> const &source,
                  InputLocation location) -> std::uint32_t {
  if (data.vertices_pm.size() >= std::numeric_limits<std::uint32_t>::max()) {
    InvalidMesh(data.source_file, "too many positions for uint32 mesh indices",
                location);
  }

  auto const index = static_cast<std::uint32_t>(data.vertices_pm.size());
  data.vertices_pm.push_back(ConvertVertex(source, data, location));

  return index;
}

/*!
 * \brief Checks triangle area after rounding its vertices to FP32.
 * \param[in] vertices Local vertices in pm.
 * \param[in] triangle Source vertex indices.
 * \param[in] file Source file for diagnostics.
 * \param[in] location Source record when available.
 */
auto RequireNondegenerate(std::span<std::array<float, 3> const> vertices,
                          std::array<std::uint32_t, 3> const &triangle,
                          std::filesystem::path const &file,
                          InputLocation location) -> void {
  auto const &origin = vertices[triangle[0]];
  std::array<double, 3> edge_a{};
  std::array<double, 3> edge_b{};

  for (std::size_t axis = 0; axis < origin.size(); ++axis) {
    edge_a[axis] = static_cast<double>(vertices[triangle[1]][axis]) -
                   static_cast<double>(origin[axis]);

    edge_b[axis] = static_cast<double>(vertices[triangle[2]][axis]) -
                   static_cast<double>(origin[axis]);
  }

  if (edge_a[1] * edge_b[2] == edge_a[2] * edge_b[1] &&
      edge_a[2] * edge_b[0] == edge_a[0] * edge_b[2] &&
      edge_a[0] * edge_b[1] == edge_a[1] * edge_b[0]) {
    InvalidMesh(file, "degenerate triangle in FP32 local geometry", location);
  }
}

/*!
 * \brief Admits a nonzero triangle and accumulates referenced local bounds.
 * \param[in,out] data Accumulated triangles, overall and component bounds.
 * \param[in] triangle Three already-checked global position indices.
 * \param[in] location Source record for zero-area failures.
 * \param[in] component Active named OBJ declaration, absent for unnamed faces.
 */
auto AppendTriangle(GGEMSMeshData &data,
                    std::array<std::uint32_t, 3> const &triangle,
                    InputLocation location,
                    std::optional<std::size_t> component = std::nullopt)
  -> void {
  RequireNondegenerate(data.vertices_pm, triangle, data.source_file, location);
  auto const &origin = data.vertices_pm[triangle[0]];

  if (data.triangles.empty()) {
    data.bounds_pm = {.minimum = origin, .maximum = origin};
  }

  if (component && !data.components[*component].bounds_pm) {
    data.components[*component].bounds_pm =
      GGEMSMeshBounds{.minimum = origin, .maximum = origin};
  }

  for (auto const index : triangle) {
    ExtendBounds(data.bounds_pm, data.vertices_pm[index]);
    if (component) {
      ExtendBounds(*data.components[*component].bounds_pm,
                   data.vertices_pm[index]);
    }
  }

  data.triangles.push_back(triangle);

  if (component) {
    ++data.components[*component].triangle_count;
  }
}

/*!
 * \brief Resolves an OBJ face token against the positions already declared.
 * \param[in] token v, v/vt, v//vn, or v/vt/vn reference.
 * \param[in] data Current global OBJ positions and diagnostic filename.
 * \param[in] location OBJ line containing the face.
 * \return Checked zero-based position index; attributes are not imported.
 */
auto ReadOBJIndex(std::string_view token, GGEMSMeshData const &data,
                  InputLocation location) -> std::uint32_t {
  auto const slash = token.find('/');
  auto const index = ReadNumber<std::int64_t>(token.substr(0, slash),
                                              data.source_file, location);
  if (slash != std::string_view::npos) {
    auto const attributes = token.substr(slash + 1U);
    auto const second_slash = attributes.find('/');
    auto const texture = attributes.substr(0, second_slash);
    if (!texture.empty()) {
      if (ReadNumber<std::int64_t>(texture, data.source_file, location) == 0) {
        InvalidMesh(data.source_file, "OBJ texture index must not be zero",
                    location);
      }
    } else if (second_slash == std::string_view::npos) {
      InvalidMesh(data.source_file, "incomplete OBJ face reference", location);
    }

    if (second_slash != std::string_view::npos &&
        ReadNumber<std::int64_t>(attributes.substr(second_slash + 1U),
                                 data.source_file, location) == 0) {
      InvalidMesh(data.source_file, "OBJ normal index must not be zero",
                  location);
    }
  }

  auto const count = static_cast<std::int64_t>(data.vertices_pm.size());

  if (index == 0 || index > count || index < -count) {
    InvalidMesh(data.source_file, "OBJ position index is zero or out of range",
                location);
  }

  return static_cast<std::uint32_t>(index > 0 ? index - 1 : count + index);
}

/*!
 * \brief Imports global OBJ positions and literal object triangle ranges.
 * \param[in,out] input Source stream positioned at its first line.
 * \param[in,out] data Import payload with initialized unit and filename.
 */
auto ReadOBJ(std::istream &input, GGEMSMeshData &data) -> void {
  std::optional<std::size_t> component;
  std::string line;

  InputLocation location;

  while (std::getline(input, line)) {
    ++location.line;
    std::string_view fields{line};
    auto const keyword = TakeToken(fields);

    if (keyword == "o") {
      auto const name = Trim(fields);
      component.reset();

      if (!name.empty()) {
        component = data.components.size();
        data.components.push_back({
          .name = std::string{name},
          .first_triangle = data.triangles.size(),
          .triangle_count = 0,
          .bounds_pm = std::nullopt,
        });
      }
      continue;
    }

    fields = fields.substr(0, fields.find('#'));

    if (keyword == "v") {
      std::array<double, 3> source{};
      for (auto &coordinate : source) {
        coordinate =
          ReadNumber<double>(TakeToken(fields), data.source_file, location);
      }

      if (!Trim(fields).empty()) {
        InvalidMesh(data.source_file,
                    "OBJ positions require exactly three coordinates",
                    location);
      }

      AppendVertex(data, source, location);
      ++data.source_vertex_count;
    } else if (keyword == "f") {
      std::array<std::uint32_t, 3> triangle{};

      for (auto &index : triangle) {
        auto const token = TakeToken(fields);
        if (token.empty()) {
          InvalidMesh(
            data.source_file,
            "OBJ faces require three vertices; triangulate the source mesh",
            location);
        }

        index = ReadOBJIndex(token, data, location);
      }

      if (!Trim(fields).empty()) {
        InvalidMesh(
          data.source_file,
          "OBJ polygon is not triangular; triangulate the source mesh",
          location);
      }

      AppendTriangle(data, triangle, location, component);
    }
  }
}

/*! \brief Hashes exact parsed STL coordinates before FP32 storage conversion.
 */
struct VertexHash {
  /*!
   * \brief Combines coordinate hashes consistently with numeric equality.
   * \param[in] vertex Finite source-coordinate triple, with signed zeros equal.
   * \return Lookup hash; iteration order never determines geometry indices.
   */
  auto operator()(std::array<double, 3> const &vertex) const noexcept
    -> std::size_t {
    std::size_t hash{};

    for (auto const coordinate : vertex) {
      hash ^= std::hash<double>{}(coordinate) + 0x9e3779b9U + (hash << 6U) +
              (hash >> 2U);
    }

    return hash;
  }
};

/*! \brief Maps exact source coordinates to first-encounter compact indices. */
using VertexIndex =
  std::unordered_map<std::array<double, 3>, std::uint32_t, VertexHash>;

/*!
 * \brief Deduplicates exact source positions in deterministic encounter order.
 * \param[in,out] data Accumulated mesh geometry and facet-corner count.
 * \param[in,out] vertices Lookup of previously encountered source coordinates.
 * \param[in] corners Three ordered facet corners in the declared source unit.
 * \param[in] location Source facet for nonfinite or degenerate input failures.
 */
auto AppendSTLFacet(GGEMSMeshData &data, VertexIndex &vertices,
                    std::array<std::array<double, 3>, 3> const &corners,
                    InputLocation location) -> void {
  std::array<std::uint32_t, 3> triangle{};
  for (std::size_t corner = 0; corner < corners.size(); ++corner) {
    auto const &source = corners[corner];

    if (!std::ranges::all_of(
          source, [](double value) -> bool { return std::isfinite(value); })) {
      InvalidMesh(data.source_file, "STL coordinates must be finite", location);
    }

    auto const found = vertices.find(source);
    if (found == vertices.end()) {
      triangle[corner] = AppendVertex(data, source, location);
      vertices.emplace(source, triangle[corner]);
    } else {
      triangle[corner] = found->second;
    }
  }

  AppendTriangle(data, triangle, location);
  data.source_vertex_count += corners.size();
}

/*!
 * \brief Decodes one little-endian uint32 without host-endian assumptions.
 * \param[in] bytes At least four readable binary STL bytes.
 * \return Unsigned value, also used as the exact bits of an FP32 coordinate.
 */
auto ReadUInt32(unsigned char const *bytes) -> std::uint32_t {
  return static_cast<std::uint32_t>(bytes[0]) |
         (static_cast<std::uint32_t>(bytes[1]) << 8U) |
         (static_cast<std::uint32_t>(bytes[2]) << 16U) |
         (static_cast<std::uint32_t>(bytes[3]) << 24U);
}

/*!
 * \brief Imports structurally sized binary facets, ignoring stored normals.
 * \param[in,out] input Source stream immediately after the binary header.
 * \param[in,out] data Import payload with initialized unit and filename.
 * \param[in] facet_count Header count validated against the complete file size.
 */
auto ReadBinarySTL(std::istream &input, GGEMSMeshData &data,
                   std::uint32_t facet_count) -> void {
  VertexIndex vertices;
  std::array<unsigned char, 50> facet{};
  for (std::uint32_t index = 0; index < facet_count; ++index) {
    InputLocation const location{.facet = static_cast<std::size_t>(index) + 1U};
    input.read(reinterpret_cast<char *>(facet.data()), facet.size());

    if (!input) {
      InvalidMesh(data.source_file, "truncated binary STL facet", location);
    }

    std::array<std::array<double, 3>, 3> corners{};
    for (std::size_t corner = 0; corner < corners.size(); ++corner) {
      for (std::size_t axis = 0; axis < corners[corner].size(); ++axis) {
        auto const offset = 12U + (((corner * 3U) + axis) * 4U);
        corners[corner][axis] =
          std::bit_cast<float>(ReadUInt32(facet.data() + offset));
      }
    }

    AppendSTLFacet(data, vertices, corners, location);
  }
}

/*!
 * \brief Reads one required ASCII STL grammar token.
 * \param[in,out] input Source stream advanced over the token.
 * \param[in] expected Literal required by the admitted facet grammar.
 * \param[in] file Source filename for structural diagnostics.
 * \param[in] location Facet currently being parsed.
 */
auto ExpectToken(std::istream &input, std::string_view expected,
                 std::filesystem::path const &file, InputLocation location)
  -> void {
  std::string token;
  if (!(input >> token) || token != expected) {
    InvalidMesh(file,
                std::format("malformed ASCII STL; expected '{}'", expected),
                location);
  }
}

/*!
 * \brief Reads one textual STL vector without using the process locale.
 * \param[in,out] input Source stream advanced over three numeric tokens.
 * \param[in] file Source filename for structural diagnostics.
 * \param[in] location Facet currently being parsed.
 * \return Three source values, also permitting advisory normal vectors.
 */
auto ReadSTLVector(std::istream &input, std::filesystem::path const &file,
                   InputLocation location) -> std::array<double, 3> {
  std::array<double, 3> vector{};
  for (auto &coordinate : vector) {
    std::string token;

    if (!(input >> token)) {
      InvalidMesh(file, "truncated ASCII STL vector", location);
    }

    coordinate = ReadNumber<double>(token, file, location);
  }

  return vector;
}

/*!
 * \brief Imports text facets from one or more solid blocks as one mesh.
 * \param[in,out] input Source stream positioned at the first solid declaration.
 * \param[in,out] data Import payload with initialized unit and filename.
 */
auto ReadAsciiSTL(std::istream &input, GGEMSMeshData &data) -> void {
  VertexIndex vertices;
  std::string token;
  while (input >> token) {
    if (token != "solid") {
      InvalidMesh(data.source_file,
                  "malformed STL: binary size mismatch or missing ASCII solid");
    }

    std::string ignored_name;
    std::getline(input, ignored_name);

    while (true) {
      InputLocation const location{.facet = data.triangles.size() + 1U};

      if (!(input >> token)) {
        InvalidMesh(data.source_file, "ASCII STL is missing endsolid",
                    location);
      }

      if (token == "endsolid") {
        std::getline(input, ignored_name);
        break;
      }

      if (token != "facet") {
        InvalidMesh(data.source_file, "malformed ASCII STL facet", location);
      }

      ExpectToken(input, "normal", data.source_file, location);
      ReadSTLVector(input, data.source_file, location);
      ExpectToken(input, "outer", data.source_file, location);
      ExpectToken(input, "loop", data.source_file, location);

      std::array<std::array<double, 3>, 3> corners{};
      for (auto &corner : corners) {
        ExpectToken(input, "vertex", data.source_file, location);
        corner = ReadSTLVector(input, data.source_file, location);
      }

      ExpectToken(input, "endloop", data.source_file, location);
      ExpectToken(input, "endfacet", data.source_file, location);
      AppendSTLFacet(data, vertices, corners, location);
    }
  }
}

/*!
 * \brief Selects binary STL by complete structural size, including solid
 * headers.
 * \param[in,out] input Seekable stream for structural detection and import.
 * \param[in,out] data Payload receiving the detected format and geometry.
 */
auto ReadSTL(std::ifstream &input, GGEMSMeshData &data) -> void {
  input.seekg(0, std::ios::end);
  auto const size = static_cast<std::streamoff>(input.tellg());
  input.seekg(0);

  std::array<unsigned char, 84> header{};
  input.read(reinterpret_cast<char *>(header.data()), header.size());

  if (input.gcount() == static_cast<std::streamsize>(header.size())) {
    auto const count = ReadUInt32(header.data() + 80);
    auto const expected_size = 84ULL + (50ULL * count);

    if (std::cmp_equal(size, expected_size)) {
      data.format = GGEMSMeshFormat::BinarySTL;
      ReadBinarySTL(input, data, count);
      return;
    }
  }

  input.clear();
  input.seekg(0);

  data.format = GGEMSMeshFormat::AsciiSTL;
  ReadAsciiSTL(input, data);
}

/*!
 * \brief Formats referenced bounds and size in automatically selected length
 * units.
 * \param[in] bounds Finite local minimum and maximum in pm.
 * \return Plain text with explicit units for one verbose summary.
 */
auto DescribeBounds(GGEMSMeshBounds const &bounds) -> std::string {
  using Coordinate = units::Quantity<units::PositionCoordinateTag, double>;
  auto const size = [&bounds](std::size_t axis) -> Coordinate {
    double const extent =
      static_cast<double>(bounds.maximum[axis]) - bounds.minimum[axis];
    return Coordinate{extent};
  };

  return std::format(
    "min=({}, {}, {}); max=({}, {}, {}); size=({}, {}, {})",
    Coordinate{bounds.minimum[0]}, Coordinate{bounds.minimum[1]},
    Coordinate{bounds.minimum[2]}, Coordinate{bounds.maximum[0]},
    Coordinate{bounds.maximum[1]}, Coordinate{bounds.maximum[2]}, size(0),
    size(1), size(2));
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSMesh::GGEMSMesh(std::filesystem::path const &mesh_file,
                     std::string_view unit)
    : GGEMSMesh({}, mesh_file, unit) {}

// -----------------------------------------------------------------------------

GGEMSMesh::GGEMSMesh(std::string name, std::filesystem::path const &mesh_file,
                     std::string_view unit)
    : name_{std::move(name)} {
  if (units::FindUnit<units::LengthUnitSet>(unit) == nullptr) {
    InvalidMesh(mesh_file, "unsupported explicit length unit");
  }

  std::ifstream input{mesh_file, std::ios::binary};
  if (!input) {
    InvalidMesh(mesh_file, "cannot open mesh file");
  }

  auto data = std::make_shared<GGEMSMeshData>();
  data->source_file = mesh_file;
  data->source_unit = unit;

  auto extension = mesh_file.extension().string();

  for (auto &character : extension) {
    if (character >= 'A' && character <= 'Z') {
      character = static_cast<char>(character - 'A' + 'a');
    }
  }

  if (extension == ".obj") {
    ReadOBJ(input, *data);
  } else if (extension == ".stl") {
    ReadSTL(input, *data);
  } else {
    InvalidMesh(mesh_file, "expected an .obj or .stl file");
  }

  if (input.bad()) {
    InvalidMesh(mesh_file, "failed while reading mesh file");
  }

  if (data->triangles.empty()) {
    InvalidMesh(mesh_file, "mesh contains no triangles");
  }
  data_ = std::move(data);
}

// -----------------------------------------------------------------------------

GGEMSMesh::GGEMSMesh(GGEMSMesh const &other)
    : GGEMSRigidPose(other),
      translations_(
        other.translations_
          ? std::make_shared<std::vector<Position3PM>>(*other.translations_)
          : nullptr),
      name_(other.name_), data_(other.data_), material_(other.material_),
      color_(other.color_) {}

// -----------------------------------------------------------------------------

auto GGEMSMesh::operator=(GGEMSMesh const &other) -> GGEMSMesh & {
  if (this != &other) {
    *this = GGEMSMesh(other);
  }

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::CaptureForAttachment() -> GGEMSMesh {
  if (!translations_) {
    translations_ = std::make_shared<std::vector<Position3PM>>();
  }

  auto captured = GGEMSRigidPose::CaptureForAttachment();
  captured.translations_ = translations_;

  return captured;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::GetComponentNames() const -> std::vector<std::string> {
  std::vector<std::string> names;
  names.reserve(data_->components.size());

  for (auto const &component : data_->components) {
    names.push_back(component.name);
  }

  return names;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::SetComponentTranslation(std::string_view name,
                                        Position3PM translation)
  -> GGEMSMesh & {
  auto const &components = data_->components;
  auto const match =
    std::ranges::find(components, name, &GGEMSMeshComponent::name);

  if (match == components.end()) {
    InvalidMesh(data_->source_file,
                std::format("no OBJ component named '{}'", name));
  }

  if (std::find_if(match + 1, components.end(),
                   [name](auto const &other) -> bool {
                     return other.name == name;
                   }) != components.end()) {
    InvalidMesh(data_->source_file,
                std::format("ambiguous OBJ component '{}'", name));
  }

  if (translation != Position3PM{}) {
    std::vector<bool> selected(data_->vertices_pm.size());
    auto const end = match->first_triangle + match->triangle_count;

    for (auto triangle = match->first_triangle; triangle < end; ++triangle) {
      for (auto vertex : data_->triangles[triangle]) {
        selected[vertex] = true;
      }
    }

    for (std::size_t index = 0; index < data_->triangles.size(); ++index) {
      if ((index < match->first_triangle || index >= end) &&
          std::ranges::any_of(
            data_->triangles[index],
            [&](auto vertex) -> bool { return selected[vertex]; })) {
        InvalidMesh(
          data_->source_file,
          std::format("OBJ component '{}' shares vertices with other faces",
                      name));
      }
    }
  }

  if (translations_ || translation != Position3PM{}) {
    if (!translations_) {
      translations_ = std::make_shared<std::vector<Position3PM>>();
    }

    auto &translations = *translations_;
    translations.resize(components.size());
    translations[static_cast<std::size_t>(match - components.begin())] =
      translation;

    if (std::ranges::all_of(translations, [](auto offset) -> bool {
          return offset == Position3PM{};
        })) {
      translations.clear();
    }
  }

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::BuildTranslatedVertices() const
  -> std::vector<std::array<float, 3>> {
  if (!translations_ || translations_->empty()) {
    return {};
  }

  auto const &translations = *translations_;

  auto vertices = data_->vertices_pm;

  for (std::size_t index = 0; index < translations.size(); ++index) {
    auto const offset = translations[index];

    if (offset == Position3PM{}) {
      continue;
    }

    auto const &component = data_->components[index];
    auto const end = component.first_triangle + component.triangle_count;

    std::array const delta{offset.x, offset.y, offset.z};

    for (auto face = component.first_triangle; face < end; ++face) {
      auto const &triangle = data_->triangles[face];
      for (auto vertex : triangle) {
        for (std::size_t axis = 0; axis < delta.size(); ++axis) {
          vertices[vertex][axis] = static_cast<float>(
            static_cast<double>(data_->vertices_pm[vertex][axis]) +
            static_cast<double>(delta[axis]));
        }
      }

      RequireNondegenerate(vertices, triangle, data_->source_file, {});
    }
  }

  return vertices;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSMesh & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Mesh material is not set.");
  }

  return *material_;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::SetColor(render::ColorKey color) noexcept -> GGEMSMesh & {
  color_ = color;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::GetColor() const noexcept -> render::ColorKey {
  return color_.value_or(material_ ? material_->GetColor()
                                   : render::WHITE_White);
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::SetPosition(Position3PM position) noexcept -> GGEMSMesh & {
  LocalPose().position_pm = position;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::SetRotation(units::Angle angle_x, units::Angle angle_y,
                            units::Angle angle_z) -> GGEMSMesh & {
  LocalPose().rotation = BuildRotationXYZ(angle_x, angle_y, angle_z);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSMesh::Verbose() -> GGEMSMesh & {
  std::string_view format{"OBJ"};

  if (data_->format == GGEMSMeshFormat::BinarySTL) {
    format = "binary STL";
  } else if (data_->format == GGEMSMeshFormat::AsciiSTL) {
    format = "ASCII STL";
  }

  auto const position = GetPositionPM();
  auto description = std::format(
    "Mesh: {}\nSource: {}\nFormat: {}; source unit: {}\nMaterial: {}\nColor: "
    "{} ({})\n"
    "Position: ({}, {}, {})\nLocal bounds: {}\n"
    "Source vertices: {}; indexed vertices: {}; triangles: {}; named objects: "
    "{}",
    name_.empty() ? "<unnamed>" : name_, data_->source_file.string(), format,
    data_->source_unit, material_ ? material_->GetName() : "<not configured>",
    render::GetColorName(GetColor()),
    color_ ? "explicit override" : "inherited",
    units::HumanReadableSignedLength(position.x),
    units::HumanReadableSignedLength(position.y),
    units::HumanReadableSignedLength(position.z),
    DescribeBounds(data_->bounds_pm), data_->source_vertex_count,
    data_->vertices_pm.size(), data_->triangles.size(),
    data_->components.size());

  for (auto const &component : data_->components) {
    description +=
      std::format("\nObject '{}': {} triangles, first {}; {}", component.name,
                  component.triangle_count, component.first_triangle,
                  component.bounds_pm ? DescribeBounds(*component.bounds_pm)
                                      : "no triangles");
  }

  GGEMS_INFO("Mesh", "{}", description);

  return *this;
}

} // namespace ggems::geometry
