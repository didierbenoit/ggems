// SPDX-License-Identifier: GPL-3.0-or-later
// Authors: Julien BERT <julien.bert@univ-brest.fr>
//          Didier BENOIT <didier.benoit@inserm.fr>
// E1 observations only: differences never become accuracy acceptance gates.
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <locale>
#include <numbers>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {
using Point = std::array<std::int64_t, 3>;
using Vec = std::array<long double, 3>;
using Trace = std::array<std::int64_t, 104>;
constexpr std::array<std::string_view, 4> profiles{"fp64", "fp32", "int64",
                                                   "int32_local"};

// Literal input fixtures copied from GGEMSSphereStreamTest.cc on 2026-10-15.
// Only inputs are retained; baseline expectations are not part of this probe.
struct SphereCase {
  std::string name;
  std::int64_t radius;
  Point local_start;
  std::array<float, 3> direction{1, 0, 0};
  Point center{};
  Point world_half{};
};

auto Cases() -> std::vector<SphereCase> {
  constexpr std::int64_t mm{1'000'000'000};
  constexpr std::int64_t cm{10 * mm};
  constexpr std::int64_t meter{1000 * mm};
  constexpr std::int64_t shift{1LL << 60};
  return {
    {.name = "rounded outward arrival",
     .radius = 1,
     .local_start = {-3, 0, 0},
     .direction = {1, 0.35F, 0},
     .center = {}},
    {.name = "mm diameter", .radius = mm, .local_start = {-2 * mm, 0, 0}},
    {.name = "mm off-axis chord",
     .radius = mm,
     .local_start = {-2 * mm, mm / 2, mm / 3}},
    {.name = "negative center",
     .radius = mm,
     .local_start = {-3 * mm, mm / 4, 0},
     .direction = {1, 0, 0},
     .center = {-15 * mm, 7 * mm, -3 * mm}},
    {.name = "cm actual nonunit D",
     .radius = cm,
     .local_start = {-2 * cm, 3 * cm / 5, 0},
     .direction = {0.75F, 0, 0}},
    {.name = "meter chord",
     .radius = meter,
     .local_start = {-2 * meter, meter / 2, meter / 4},
     .direction = {0.5F, 0, 0}},
    {.name = "oblique stored D",
     .radius = 2 * cm,
     .local_start = {-4 * cm, -2 * cm, 0},
     .direction = {0.5F, 0.25F, 0.125F}},
    {.name = "float norm not one",
     .radius = mm - 1,
     .local_start = {-3 * mm, 0, 0},
     .direction = {0x1.000002p0F, 0, 0}},
    {.name = "miss by one pm",
     .radius = mm,
     .local_start = {-2 * mm, mm + 1, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "exact tangent",
     .radius = mm,
     .local_start = {-2 * mm, mm, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "nearby positive chord",
     .radius = mm,
     .local_start = {-2 * mm, mm - 1, 0}},
    {.name = "center birth",
     .radius = cm,
     .local_start = {0, 0, 0},
     .direction = {0.5F, 0.25F, 0.125F},
     .center = {}},
    {.name = "surface inward",
     .radius = mm,
     .local_start = {mm, 0, 0},
     .direction = {-1, 0, 0},
     .center = {}},
    {.name = "surface outward",
     .radius = mm,
     .local_start = {mm, 0, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "surface tangent",
     .radius = mm,
     .local_start = {0, mm, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "just outside",
     .radius = cm,
     .local_start = {cm + 1, 0, 0},
     .direction = {-1, 0, 0}},
    {.name = "just inside",
     .radius = cm,
     .local_start = {cm - 1, 0, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "negative direction",
     .radius = mm,
     .local_start = {2 * mm, 0, 0},
     .direction = {-0.5F, 0, 0}},
    {.name = "entry rounds to same point",
     .radius = 3,
     .local_start = {3, 1, 0},
     .direction = {-1, 0, 0},
     .center = {}},
    {.name = "one pm radius",
     .radius = 1,
     .local_start = {-2, 0, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "odd pm radius",
     .radius = 7,
     .local_start = {-20, 2, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "large common positive origin",
     .radius = mm + 3,
     .local_start = {-3 * mm, mm / 2, 0},
     .direction = {1, 0, 0},
     .center = {shift, shift, -shift}},
    {.name = "large common negative origin",
     .radius = mm,
     .local_start = {2 * mm, mm / 4, 0},
     .direction = {-0.75F, 0, 0},
     .center = {-shift, shift, -shift}},
    {.name = "small oblique surface inward",
     .radius = 5,
     .local_start = {3, 4, 0},
     .direction = {-1, 0, 0},
     .center = {}},
    {.name = "medical oblique surface inward",
     .radius = 5 * cm,
     .local_start = {3 * cm, 4 * cm, 0},
     .direction = {-1, 0, 0}},
    {.name = "medical oblique surface outward",
     .radius = 5 * cm,
     .local_start = {3 * cm, 4 * cm, 0}},
    {.name = "medical oblique surface tangent",
     .radius = 5 * cm,
     .local_start = {3 * cm, 4 * cm, 0},
     .direction = {-4, 3, 0}},
    {.name = "sub-pm radial outside birth",
     .radius = mm,
     .local_start = {mm, 1, 0},
     .direction = {-1, 0, 0}},
    {.name = "compensated integer profile limit",
     .radius = 1LL << 50,
     .local_start = {1LL << 50, 1, 0},
     .direction = {-1, 0, 0}},
    {.name = "unresolved rounded grazing chord",
     .radius = 1,
     .local_start = {-2, 0, 0},
     .direction = {1, 0.49F, 0}},
    {.name = "unresolved extreme grazing",
     .radius = 1LL << 55,
     .local_start = {-(1LL << 56), (1LL << 55) - 1, 0},
     .direction = {1, 0, 0},
     .center = {}},
    {.name = "unresolved extreme radial birth",
     .radius = 1LL << 55,
     .local_start = {1LL << 55, 1, 0},
     .direction = {-1, 0, 0},
     .center = {}},
    {.name = "FP32 discriminant range observation",
     .radius = 4'000'000'000'000'000'000LL,
     .local_start = {0, 0, 0},
     .direction = {-4, 3, 0},
     .world_half = {4'500'000'000'000'000'000LL, 4'500'000'000'000'000'000LL,
                    4'500'000'000'000'000'000LL}},
    {.name = "INT32 local position domain edge plus one pm",
     .radius = mm,
     .local_start = {(32767LL << 24) + (1LL << 23), 0, 0},
     .direction = {-1, 0, 0},
     .world_half = {meter, meter, meter}},
    {.name = "integer direction above four",
     .radius = mm,
     .local_start = {-2 * mm, 0, 0},
     .direction = {0x1.000002p2F, 0, 0}},
    {.name = "integer direction quantizes to zero",
     .radius = mm,
     .local_start = {-2 * mm, 0, 0},
     .direction = {0x1p-20F, 0, 0}},
  };
}

auto Check(cl_int status, std::string_view operation) -> void {
  if (status != CL_SUCCESS) {
    throw std::runtime_error(std::string(operation) + ": OpenCL error " +
                             std::to_string(status));
  }
}

auto Quote(std::string_view value) -> std::string {
  std::ostringstream result;
  result << '"';
  for (char const byte : value) {
    auto const character = static_cast<unsigned char>(byte);
    if (character == '"' || character == '\\') {
      result << '\\' << character;
    } else if (character < 0x20) {
      result << "\\u" << std::hex << std::setw(4) << std::setfill('0')
             << static_cast<unsigned int>(character) << std::dec;
    } else {
      result << character;
    }
  }
  result << '"';
  return result.str();
}

template <typename T> auto Number(T value) -> std::string {
  std::ostringstream result;
  result.imbue(std::locale::classic());
  result << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
  return result.str();
}

auto Numeric(long double value) -> std::string {
  return std::isfinite(value) ? Number(value) : "null";
}

auto Boolean(bool value) -> std::string { return value ? "true" : "false"; }

using Field = std::pair<std::string_view, std::string>;
auto Row(std::ostream &output, std::string_view kind,
         std::initializer_list<Field> fields) -> void {
  output << "{\"kind\":" << Quote(kind);
  for (auto const &[name, value] : fields) {
    output << ',' << Quote(name) << ':' << value;
  }
  output << "}\n";
  output.flush();
}

template <typename T>
auto Array(std::array<T, 3> const &values) -> std::string {
  // Integer coordinates are JSON strings to survive binary64-only readers.
  return '[' + Quote(Number(values[0])) + ',' + Quote(Number(values[1])) + ',' +
         Quote(Number(values[2])) + ']';
}

auto Difference(std::int64_t actual, std::int64_t expected) -> std::string {
  auto const a = static_cast<std::uint64_t>(actual);
  auto const b = static_cast<std::uint64_t>(expected);
  return actual >= expected ? Number(a - b) : '-' + Number(b - a);
}

auto Magnitude(std::int64_t actual, std::int64_t expected) -> long double {
  auto const a = static_cast<std::uint64_t>(actual);
  auto const b = static_cast<std::uint64_t>(expected);
  return static_cast<long double>(actual >= expected ? a - b : b - a);
}

auto Parameter(std::size_t profile, Trace const &trace, std::size_t offset)
  -> long double {
  if (profile >= 2) {
    auto const extra = 65 + (13 * ((offset - 8) / 16));
    return std::ldexp(
      static_cast<long double>(std::bit_cast<std::uint64_t>(trace[extra + 1])) /
        static_cast<long double>(
          std::bit_cast<std::uint64_t>(trace[extra + 2])),
      static_cast<int>(trace[extra + 3]));
  }
  if (profile == 1) {
    return std::bit_cast<float>(static_cast<std::uint32_t>(trace[offset + 7]));
  }
  return std::bit_cast<double>(trace[offset + 7]);
}

// Independent closest-point/chord oracle from the existing Sphere fixture.
// This is extended host arithmetic, not an exact sign certificate at extremes.
auto Oracle(SphereCase const &fixture, Point const &start, bool inside)
  -> std::optional<long double> {
  Vec local{};
  Vec direction{};
  long double norm2{};
  long double projection{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    local[axis] = static_cast<long double>(start[axis] - fixture.center[axis]);
    direction[axis] = fixture.direction[axis];
    norm2 += direction[axis] * direction[axis];
    projection += local[axis] * direction[axis];
  }
  auto const closest = -projection / norm2;
  long double impact2{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    auto const transverse = local[axis] + (closest * direction[axis]);
    impact2 += transverse * transverse;
  }
  auto const radius = static_cast<long double>(fixture.radius);
  auto const half_squared = ((radius * radius) - impact2) / norm2;
  if (!(half_squared >= 0)) {
    return std::nullopt;
  }
  auto const half = std::sqrt(half_squared);
  return inside ? closest + half : closest - half;
}

template <typename T>
auto InputBuffer(cl::Context const &context, T &value) -> cl::Buffer {
  cl_int error{};
  cl::Buffer buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(T),
                    &value, &error);
  Check(error, "input buffer");
  return buffer;
}

auto CaptureCone(ggems::ocl::GGEMSOpenCLContext &context,
                 cl::Program const &program, std::vector<SphereCase> &fixtures,
                 std::ostream &output, std::size_t device) -> void {
  ggems::core::sources::GGEMSSourceRecord source;
  source.axis_x_x = 0;
  source.axis_x_y = 1;
  source.axis_y_y = 0;
  source.axis_y_z = 1;
  source.axis_z_x = 1;
  source.axis_z_z = 0;
  source.angular_distribution_type =
    ggems::core::sources::ToKernelAngularDistributionType(
      ggems::core::sources::GGEMSAngularDistributionType::Isotropic);
  source.isotropic_cos_theta_lower =
    static_cast<float>(std::cos(2.5 * std::numbers::pi / 180));
  source.isotropic_cos_theta_upper = 1;
  // Fixed stratified uniforms sample the production solid-angle law. They are
  // captured once; no profile receives a separately sampled direction.
  std::array<float, 32> uniforms{};
  constexpr std::array<float, 8> azimuth{0.03125F, 0.59375F, 0.28125F,
                                         0.84375F, 0.15625F, 0.71875F,
                                         0.40625F, 0.96875F};
  for (std::size_t index = 0; index < 8; ++index) {
    uniforms[4 * index] = (static_cast<float>(index) + 0.5F) / 8;
    uniforms[(4 * index) + 1] = azimuth[index];
  }
  auto source_buffer = InputBuffer(context.GetContextNative(), source);
  auto uniform_buffer = InputBuffer(context.GetContextNative(), uniforms);
  std::array<float, 32> directions{};
  cl_int error{};
  cl::Buffer direction_buffer(context.GetContextNative(), CL_MEM_WRITE_ONLY,
                              sizeof(directions), nullptr, &error);
  Check(error, "capture direction buffer");
  cl::Kernel kernel(program, "sphere_precision_capture", &error);
  Check(error, "capture kernel");
  Check(kernel.setArg(0, source_buffer), "capture source");
  Check(kernel.setArg(1, uniform_buffer), "capture uniforms");
  Check(kernel.setArg(2, direction_buffer), "capture output");
  auto const &queue = context.GetCommandQueueNative();
  Check(queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(8),
                                   cl::NDRange(1)),
        "capture dispatch");
  Check(queue.enqueueReadBuffer(direction_buffer, CL_TRUE, 0,
                                sizeof(directions), directions.data()),
        "capture read");
  for (std::size_t index = 0; index < 8; ++index) {
    SphereCase fixture{.name = "captured medical cone " + Number(index),
                       .radius = 250'000'000'000LL,
                       .local_start = {-500'000'000'000LL, 0, 0},
                       .direction = {directions[4 * index],
                                     directions[(4 * index) + 1],
                                     directions[(4 * index) + 2]},
                       .world_half = {1'500'000'000'000LL, 1'500'000'000'000LL,
                                      1'500'000'000'000LL}};
    Row(
      output, "source_capture",
      {{"device", Number(device)},
       {"ray", Quote(fixture.name)},
       {"uniform_theta_bits",
        Quote(Number(std::bit_cast<std::uint32_t>(uniforms[4 * index])))},
       {"uniform_phi_bits",
        Quote(Number(std::bit_cast<std::uint32_t>(uniforms[(4 * index) + 1])))},
       {"cos_theta_lower_bits", Quote(Number(std::bit_cast<std::uint32_t>(
                                  source.isotropic_cos_theta_lower)))},
       {"sampling", Quote("fixed stratified uniforms; production "
                          "SourceSampleDirectionFromUniforms; +X frame with "
                          "Xlocal=+Y, Ylocal=+Z; azimuth [0,2pi)")}});
    fixtures.push_back(std::move(fixture));
  }
}

auto WorldHalf(SphereCase const &fixture) -> Point {
  Point half{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    half[axis] =
      fixture.world_half[axis] != 0
        ? fixture.world_half[axis]
        : std::abs(fixture.center[axis]) + (4 * fixture.radius) + 100;
  }
  return half;
}

auto Execute(ggems::ocl::GGEMSOpenCLContext &context,
             cl::Program const &program, SphereCase const &fixture) -> Trace {
  auto const half = WorldHalf(fixture);
  ggems::geometry::GGEMSWorldRecord world{.half_extent_x_pm = half[0],
                                          .half_extent_y_pm = half[1],
                                          .half_extent_z_pm = half[2]};
  ggems::geometry::GGEMSAnalyticVolumeRecord volume;
  volume.kind = ggems::geometry::GGEMSAnalyticKind::Sphere;
  volume.parameters.sphere = {.center_x_pm = fixture.center[0],
                              .center_y_pm = fixture.center[1],
                              .center_z_pm = fixture.center[2],
                              .radius_pm =
                                static_cast<std::uint64_t>(fixture.radius),
                              .volume_id = 1U,
                              .material_id = 7U};
  ggems::core::particles::GGEMSParticleState particle;
  particle.position_x_pm = fixture.center[0] + fixture.local_start[0];
  particle.position_y_pm = fixture.center[1] + fixture.local_start[1];
  particle.position_z_pm = fixture.center[2] + fixture.local_start[2];
  particle.direction_x = fixture.direction[0];
  particle.direction_y = fixture.direction[1];
  particle.direction_z = fixture.direction[2];
  particle.energy_micro_eV = 123456789;
  particle.time_ps = 17;
  particle.particle_type = ggems::core::particles::ToKernelParticleType(
    ggems::core::particles::GGEMSParticleType::Aionino);
  particle.status = ggems::core::particles::ToKernelParticleStatus(
    ggems::core::particles::GGEMSParticleStatus::Alive);
  auto world_buffer = InputBuffer(context.GetContextNative(), world);
  auto volume_buffer = InputBuffer(context.GetContextNative(), volume);
  auto input_buffer = InputBuffer(context.GetContextNative(), particle);
  Trace trace{};
  cl_int error{};
  cl::Buffer trace_buffer(context.GetContextNative(), CL_MEM_WRITE_ONLY,
                          sizeof(trace), nullptr, &error);
  Check(error, "trace buffer");
  cl::Kernel kernel(program, "sphere_precision_probe", &error);
  Check(error, "probe kernel");
  Check(kernel.setArg(0, world_buffer), "world argument");
  Check(kernel.setArg(1, volume_buffer), "volume argument");
  Check(kernel.setArg(2, input_buffer), "particle argument");
  Check(kernel.setArg(3, trace_buffer), "trace argument");
  auto const &queue = context.GetCommandQueueNative();
  Check(queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(1),
                                   cl::NDRange(1)),
        "probe dispatch");
  Check(queue.enqueueReadBuffer(trace_buffer, CL_TRUE, 0, sizeof(trace),
                                trace.data()),
        "trace read");
  return trace;
}

struct ErrorSamples {
  std::vector<long double> parameters, endpoints, paths;
  std::size_t parameter_rays{}, endpoint_rays{}, path_rays{};
};

auto PathLength(SphereCase const &fixture, Trace const &trace,
                std::size_t offset, std::size_t profile) -> long double {
  long double norm2{};
  auto const extra = 65 + (13 * ((offset - 8) / 16));
  for (std::size_t axis = 0; axis < 3; ++axis) {
    auto const component =
      profile >= 2 && trace[offset + 2] != 1
        ? std::ldexp(static_cast<long double>(trace[extra + 9 + axis]),
                     -static_cast<int>(trace[extra + 3]))
        : static_cast<long double>(fixture.direction[axis]);
    norm2 += component * component;
  }
  return Parameter(profile, trace, offset) * std::sqrt(norm2);
}

auto Quantization(std::ostream &output, SphereCase const &fixture,
                  Point const &start, std::size_t device, std::size_t profile,
                  std::int64_t attempt, std::int64_t reason, Point const &local,
                  std::int64_t radius, Point const &direction, bool used)
  -> void {
  auto const spatial_shift = profile == 3 ? 24 : 0;
  auto const direction_shift = profile == 3 ? 12 : 17;
  Point position_error{};
  Vec direction_error{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (reason == 0) {
      position_error[axis] = (local[axis] * (1LL << spatial_shift)) -
                             (start[axis] - fixture.center[axis]);
    }
    direction_error[axis] =
      std::ldexp(static_cast<long double>(direction[axis]), -direction_shift) -
      fixture.direction[axis];
  }
  Row(
    output, "quantization",
    {{"device", Number(device)},
     {"ray", Quote(fixture.name)},
     {"profile", Quote(profiles[profile])},
     {"attempt", Number(attempt)},
     {"used_by_query", Boolean(used)},
     {"reason", Number(reason)},
     {"spatial_quantum_pm", Quote(Number(1LL << spatial_shift))},
     {"direction_quantum", Numeric(std::ldexp(1.L, -direction_shift))},
     {"local_position_ticks", reason == 0 ? Array(local) : "null"},
     {"local_radius_ticks", reason == 0 ? Quote(Number(radius)) : "null"},
     {"integer_direction", reason == 0 ? Array(direction) : "null"},
     {"position_difference_pm", reason == 0 ? Array(position_error) : "null"},
     {"radius_difference_pm",
      reason == 0
        ? Quote(Number((radius * (1LL << spatial_shift)) - fixture.radius))
        : "null"},
     {"direction_difference", reason == 0 ? Array(direction_error) : "null"}});
}

auto EmitTrace(std::ostream &output, SphereCase const &fixture,
               Trace const &trace, std::size_t device,
               std::size_t profile_index, ErrorSamples &errors) -> void {
  auto const profile = Quote(profiles[profile_index]);
  auto const initial_parameter_count = errors.parameters.size();
  auto const initial_endpoint_count = errors.endpoints.size();
  bool hit = trace[1] == 1;
  bool world_exit{};
  long double cumulative_path{};
  if (profile_index >= 2) {
    Point start{};
    for (std::size_t axis = 0; axis < 3; ++axis) {
      start[axis] = fixture.center[axis] + fixture.local_start[axis];
    }
    Quantization(output, fixture, start, device, profile_index, -1, trace[57],
                 {trace[58], trace[59], trace[60]}, trace[61],
                 {trace[62], trace[63], trace[64]}, true);
  }
  for (std::int64_t attempt = 0; attempt < trace[4]; ++attempt) {
    auto const offset = 8 + (16 * static_cast<std::size_t>(attempt));
    auto const extra = 65 + (13 * static_cast<std::size_t>(attempt));
    bool const accepted = trace[offset + 15] == 1;
    hit |= accepted && trace[offset + 2] == 2;
    world_exit |= accepted && trace[offset + 2] == 1;
    Point const start{trace[offset + 8], trace[offset + 9], trace[offset + 10]};
    Point const endpoint{trace[offset + 11], trace[offset + 12],
                         trace[offset + 13]};
    bool const queried = trace[offset] == 0;
    auto const parameter =
      queried ? Parameter(profile_index, trace, offset) : 0;
    auto const path =
      queried ? PathLength(fixture, trace, offset, profile_index) : 0;
    if (accepted) {
      cumulative_path += path;
    }
    if (profile_index >= 2) {
      bool const used = attempt == 0 || trace[offset - 16 + 2] != 3;
      Quantization(output, fixture, start, device, profile_index, attempt,
                   trace[extra + 12],
                   {trace[extra + 5], trace[extra + 6], trace[extra + 7]},
                   trace[extra + 8],
                   {trace[extra + 9], trace[extra + 10], trace[extra + 11]},
                   used);
    }
    std::optional<long double> oracle;
    if (queried && (trace[offset + 2] == 2 || trace[offset + 2] == 3)) {
      oracle = Oracle(fixture, start, trace[offset + 4] == 1);
    }
    std::string oracle_endpoint = "null";
    if (accepted && oracle) {
      auto const parameter_error = parameter - *oracle;
      if (std::isfinite(parameter_error)) {
        errors.parameters.push_back(std::abs(parameter_error));
      }
      Vec endpoint_error{};
      for (std::size_t axis = 0; axis < 3; ++axis) {
        auto const observed_local = endpoint[axis] - fixture.center[axis];
        auto const start_local = start[axis] - fixture.center[axis];
        endpoint_error[axis] = static_cast<long double>(observed_local) -
                               (static_cast<long double>(start_local) +
                                (*oracle * fixture.direction[axis]));
        if (std::isfinite(endpoint_error[axis])) {
          errors.endpoints.push_back(std::abs(endpoint_error[axis]));
        }
      }
      oracle_endpoint = '[' + Numeric(endpoint_error[0]) + ',' +
                        Numeric(endpoint_error[1]) + ',' +
                        Numeric(endpoint_error[2]) + ']';
    }
    Row(output, "attempt",
        {{"device", Number(device)},
         {"ray", Quote(fixture.name)},
         {"profile", profile},
         {"attempt", Number(attempt)},
         {"query_status", Number(trace[offset])},
         {"transport_status", Number(trace[offset + 1])},
         {"accepted", Boolean(accepted)},
         {"event", Number(trace[offset + 2])},
         {"faces", Number(trace[offset + 3])},
         {"departure_owner", Number(trace[offset + 4])},
         {"arrival_owner", Number(trace[offset + 5])},
         {"material", Number(trace[offset + 6])},
         {"start_pm", Array(start)},
         {"endpoint_pm", queried ? Array(endpoint) : "null"},
         {"parameter_pm", queried ? Numeric(parameter) : "null"},
         {"path_length_pm", queried ? Numeric(path) : "null"},
         {"cumulative_accepted_path_length_pm", Numeric(cumulative_path)},
         {"integer_query_reason",
          profile_index >= 2 ? Number(trace[extra]) : "null"},
         {"integer_parameter_numerator_pm",
          profile_index >= 2 && queried
            ? Quote(Number(std::bit_cast<std::uint64_t>(trace[extra + 1])))
            : "null"},
         {"integer_parameter_denominator",
          profile_index >= 2 && queried
            ? Quote(Number(std::bit_cast<std::uint64_t>(trace[extra + 2])))
            : "null"},
         {"integer_parameter_binary_exponent",
          profile_index >= 2 && queried ? Number(trace[extra + 3]) : "null"},
         {"parameter_bits",
          queried && profile_index < 2
            ? Quote(Number(std::bit_cast<std::uint64_t>(trace[offset + 7])))
            : "null"},
         {"oracle_parameter_pm", oracle ? Numeric(*oracle) : "null"},
         {"oracle_parameter_difference_pm",
          oracle ? Numeric(parameter - *oracle) : "null"},
         {"oracle_endpoint_difference_pm", oracle_endpoint}});
  }
  Row(output, "ray",
      {{"device", Number(device)},
       {"ray", Quote(fixture.name)},
       {"profile", profile},
       {"radius_pm", Quote(Number(fixture.radius))},
       {"center_pm", Array(fixture.center)},
       {"world_half_pm", Array(WorldHalf(fixture))},
       {"local_start_pm", Array(fixture.local_start)},
       {"direction", Array(fixture.direction)},
       {"direction_bits",
        Array(std::array{std::bit_cast<std::uint32_t>(fixture.direction[0]),
                         std::bit_cast<std::uint32_t>(fixture.direction[1]),
                         std::bit_cast<std::uint32_t>(fixture.direction[2])})},
       {"locate_status", Number(trace[0])},
       {"initial_owner", Number(trace[1])},
       {"initial_material", Number(trace[56])},
       {"final_navigation_status", Number(trace[2])},
       {"accepted_events", Number(trace[3])},
       {"attempts", Number(trace[4])},
       {"failure_stage", Number(trace[5])},
       {"direction_energy_time_unchanged", Boolean(trace[6] == 1)},
       {"particle_status", Number(trace[7])},
       {"geometric_hit", hit || world_exit ? Boolean(hit) : "null"},
       {"world_exit", Boolean(world_exit)},
       {"unresolved", Boolean(trace[2] == 2)}});
  errors.parameter_rays +=
    errors.parameters.size() != initial_parameter_count ? 1U : 0U;
  errors.endpoint_rays +=
    errors.endpoints.size() != initial_endpoint_count ? 1U : 0U;
}

struct Summary {
  std::size_t rays{}, category_disagreements{}, fp64_unresolved{},
    candidate_unresolved{};
  std::array<ErrorSamples, 4> paired;
};

auto Compare(std::ostream &output, SphereCase const &fixture,
             Trace const &reference, Trace const &candidate, std::size_t device,
             std::size_t profile_index, Summary &summary) -> void {
  ++summary.rays;
  summary.fp64_unresolved += reference[2] == 2 ? 1U : 0U;
  summary.candidate_unresolved += candidate[2] == 2 ? 1U : 0U;
  bool event_order{};
  bool ownership{};
  bool material{};
  bool faces{};
  std::array<bool, 2> hit{};
  std::array<bool, 2> exited{};
  std::array<std::array<std::size_t, 3>, 4> initial_counts{};
  for (std::size_t group = 0; group < 4; ++group) {
    initial_counts[group] = {summary.paired[group].parameters.size(),
                             summary.paired[group].endpoints.size(),
                             summary.paired[group].paths.size()};
  }
  long double cumulative_path_difference{};
  bool matching_prefix = reference[0] == candidate[0] &&
                         reference[1] == candidate[1] &&
                         reference[56] == candidate[56];
  hit[0] = reference[1] == 1;
  hit[1] = candidate[1] == 1;
  for (std::size_t index = 0; index < 3; ++index) {
    auto const offset = 8 + (16 * index);
    bool const accepted64 = reference[offset + 15] == 1;
    bool const accepted32 = candidate[offset + 15] == 1;
    event_order |= accepted64 != accepted32;
    if (accepted64) {
      hit[0] |= reference[offset + 2] == 2;
      exited[0] |= reference[offset + 2] == 1;
    }
    if (accepted32) {
      hit[1] |= candidate[offset + 2] == 2;
      exited[1] |= candidate[offset + 2] == 1;
    }
    if (!(accepted64 && accepted32)) {
      matching_prefix = false;
      continue;
    }
    event_order |= reference[offset + 2] != candidate[offset + 2];
    ownership |= reference[offset + 4] != candidate[offset + 4] ||
                 reference[offset + 5] != candidate[offset + 5];
    material |= reference[offset + 6] != candidate[offset + 6];
    faces |= reference[offset + 3] != candidate[offset + 3];
    bool const matching = matching_prefix &&
                          reference[offset + 2] == candidate[offset + 2] &&
                          reference[offset + 4] == candidate[offset + 4] &&
                          reference[offset + 5] == candidate[offset + 5] &&
                          reference[offset + 6] == candidate[offset + 6];
    matching_prefix = matching;
    if (!matching) {
      continue;
    }
    auto const difference = Parameter(profile_index, candidate, offset) -
                            Parameter(0, reference, offset);
    auto const path_difference =
      PathLength(fixture, candidate, offset, profile_index) -
      PathLength(fixture, reference, offset, 0);
    cumulative_path_difference += path_difference;
    std::array<std::string, 3> delta;
    bool same_anchor{true};
    for (std::size_t axis = 0; axis < 3; ++axis) {
      delta[axis] = Difference(candidate[offset + 11 + axis],
                               reference[offset + 11 + axis]);
      same_anchor &=
        candidate[offset + 8 + axis] == reference[offset + 8 + axis];
    }
    auto const group =
      static_cast<std::size_t>(reference[offset + 2] == 1 ? 2 : 0) +
      (same_anchor ? 0U : 1U);
    if (std::isfinite(difference)) {
      summary.paired[group].parameters.push_back(std::abs(difference));
    }
    if (std::isfinite(path_difference)) {
      summary.paired[group].paths.push_back(std::abs(path_difference));
    }
    for (std::size_t axis = 0; axis < 3; ++axis) {
      summary.paired[group].endpoints.push_back(Magnitude(
        candidate[offset + 11 + axis], reference[offset + 11 + axis]));
    }
    Row(output, "paired_event",
        {{"device", Number(device)},
         {"ray", Quote(fixture.name)},
         {"attempt", Number(index)},
         {"event", Number(reference[offset + 2])},
         {"profile", Quote(profiles[profile_index])},
         {"reference", Quote(profiles[0])},
         {"identical_start_anchor", Boolean(same_anchor)},
         {"candidate_minus_fp64_parameter_pm", Numeric(difference)},
         {"candidate_minus_fp64_path_length_pm", Numeric(path_difference)},
         {"candidate_minus_fp64_cumulative_path_length_pm",
          Numeric(cumulative_path_difference)},
         {"candidate_minus_fp64_endpoint_pm", '[' + Quote(delta[0]) + ',' +
                                                Quote(delta[1]) + ',' +
                                                Quote(delta[2]) + ']'}});
  }
  for (std::size_t group = 0; group < 4; ++group) {
    summary.paired[group].parameter_rays +=
      summary.paired[group].parameters.size() != initial_counts[group][0] ? 1U
                                                                          : 0U;
    summary.paired[group].endpoint_rays +=
      summary.paired[group].endpoints.size() != initial_counts[group][1] ? 1U
                                                                         : 0U;
    summary.paired[group].path_rays +=
      summary.paired[group].paths.size() != initial_counts[group][2] ? 1U : 0U;
  }
  bool const locate = reference[0] != candidate[0];
  bool const initial = reference[1] != candidate[1];
  bool const status = reference[2] != candidate[2];
  bool const failure = reference[5] != candidate[5];
  bool const count = reference[3] != candidate[3];
  bool const attempts = reference[4] != candidate[4];
  bool const initial_material = reference[56] != candidate[56];
  bool const unchanged = reference[6] != candidate[6];
  bool const particle = reference[7] != candidate[7];
  bool const both_hit_known = (hit[0] || exited[0]) && (hit[1] || exited[1]);
  bool const hit_mismatch = both_hit_known && hit[0] != hit[1];
  bool const hit_availability = (hit[0] || exited[0]) != (hit[1] || exited[1]);
  bool const any = locate || initial || initial_material || status || failure ||
                   count || attempts || unchanged || particle || event_order ||
                   ownership || material || faces || hit_mismatch ||
                   hit_availability || exited[0] != exited[1];
  summary.category_disagreements += any ? 1U : 0U;
  Row(output, "paired_categories",
      {{"device", Number(device)},
       {"ray", Quote(fixture.name)},
       {"profile", Quote(profiles[profile_index])},
       {"reference", Quote(profiles[0])},
       {"any_disagreement", Boolean(any)},
       {"locate_status", Boolean(locate)},
       {"initial_owner", Boolean(initial)},
       {"initial_material", Boolean(initial_material)},
       {"final_status", Boolean(status)},
       {"failure_stage", Boolean(failure)},
       {"attempt_count", Boolean(attempts)},
       {"event_count", Boolean(count)},
       {"event_order", Boolean(event_order)},
       {"ownership", Boolean(ownership)},
       {"material", Boolean(material)},
       {"face_mask", Boolean(faces)},
       {"hit_miss", both_hit_known ? Boolean(hit_mismatch) : "null"},
       {"hit_decision_availability", Boolean(hit_availability)},
       {"world_exit", Boolean(exited[0] != exited[1])},
       {"state_preservation", Boolean(unchanged)},
       {"particle_status", Boolean(particle)}});
}

auto Distribution(std::ostream &output, std::size_t device,
                  std::string_view metric, std::vector<long double> values,
                  std::size_t contributing_rays, std::size_t total_rays,
                  std::size_t profile) -> void {
  std::ranges::sort(values);
  auto const count = values.size();
  long double median{};
  if (count != 0) {
    median = count % 2 == 0 ? (values[(count / 2) - 1] + values[count / 2]) / 2
                            : values[count / 2];
  }
  Row(output, "distribution",
      {{"device", Number(device)},
       {"metric", Quote(metric)},
       {"profile", Quote(profiles[profile])},
       {"contributing_rays", Number(contributing_rays)},
       {"excluded_rays", Number(total_rays - contributing_rays)},
       {"count", Number(count)},
       {"minimum_pm", count != 0 ? Numeric(values.front()) : "null"},
       {"median_pm", count != 0 ? Numeric(median) : "null"},
       {"p95_pm",
        count != 0 ? Numeric(values[(((95 * count) + 99) / 100) - 1]) : "null"},
       {"p99_pm",
        count != 0 ? Numeric(values[(((99 * count) + 99) / 100) - 1]) : "null"},
       {"percentile_definition", Quote("nearest-rank ceil(p * count)")},
       {"maximum_pm", count != 0 ? Numeric(values.back()) : "null"}});
}

auto Run(std::string const &selector, std::ostream &output) -> int {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  opencl.SelectDevices({selector});
  opencl.Initialize();
  std::ifstream input(GGEMS_E1_PROBE_PATH, std::ios::binary);
  if (!input) {
    throw std::runtime_error("cannot read E1 probe source");
  }
  std::string const source{std::istreambuf_iterator<char>(input),
                           std::istreambuf_iterator<char>()};
  Row(output, "configuration",
      {{"schema", Quote("ggems-sphere-precision-e2-v1")},
       {"selector", Quote(selector)},
       {"build_configuration", Quote(GGEMS_E1_BUILD_CONFIGURATION)},
       {"host_reference_significand_bits",
        Number(std::numeric_limits<long double>::digits)},
       {"probe_global_size", "1"},
       {"probe_local_size", "1"},
       {"parameter_provenance",
        Quote("Diagnostic pre-query; Transport reissues the query internally "
              "before commit. World parameters are reconstructed observations, "
              "unused by exact planar movement.")},
       {"failure_stage_codes",
        Quote("0=none, 1=locate, 2=diagnostic query, 3=Transport, "
              "4=three-event budget exhausted")},
       {"integer_reason_codes",
        Quote("0=ok,1=position domain,2=radius domain,3=direction domain,"
              "4=zero quantized direction,5=ownership,6=root,7=World order;"
              "reason is diagnostic pre-query, not Transport's "
              "repeated query")},
       {"integer_parameter_definition",
        Quote(
          "t_pm=numerator_pm/denominator*2^binary_exponent; integer Sphere "
          "displacement uses its quantized direction; World uses the stored "
          "direction. Path length is t times the applicable direction norm.")},
       {"statistic_exclusion_definition",
        Quote(
          "excluded_rays = all input rays minus rays with at least one finite "
          "sample for that statistic; count is scalar samples, not rays")},
       {"program_origin",
        Quote("uncached cl::Program built directly from source; vendor "
              "internal caching is not observable")},
       {"oracle",
        Quote("Independent closest-point/chord long-double arithmetic at each "
              "profile's own canonical anchor. Approximate: extreme "
              "cancellation and 1-pm-squared residuals may be unresolved; null "
              "is unavailable, never a passed check.")},
       {"probe_source", Quote(source)}});
  std::size_t unavailable{};

  for (std::size_t device_index = 0; device_index < opencl.GetContext().size();
       ++device_index) {
    auto &context = opencl.GetContext()[device_index];
    auto const &device = context.GetDevice();
    cl_int error{};
    cl::Platform platform(device.GetPlatformID());
    auto const runtime = platform.getInfo<CL_PLATFORM_VERSION>(&error);
    Check(error, "platform version");
    Row(output, "device",
        {{"index", Number(device_index)},
         {"name", Quote(device.GetName())},
         {"vendor", Quote(device.GetVendor())},
         {"driver", Quote(device.GetDriverVersion())},
         {"opencl_version", Quote(device.GetVersion())},
         {"runtime_version", Quote(runtime)},
         {"little_endian", Boolean(device.GetEndianLittle() != CL_FALSE)}});
    if ((std::endian::native != std::endian::little &&
         std::endian::native != std::endian::big) ||
        (device.GetEndianLittle() != CL_FALSE) !=
          (std::endian::native == std::endian::little)) {
      ++unavailable;
      Row(output, "unsupported",
          {{"device", Number(device_index)},
           {"reason",
            Quote(
              "Raw shared records require matching host/device byte order")}});
      continue;
    }
    if (device.GetDoubleFpConfig() == 0) {
      ++unavailable;
      Row(output, "unsupported",
          {{"device", Number(device_index)},
           {"reason", Quote("FP64 reference unavailable; this device cannot "
                            "provide a paired comparison")}});
      continue;
    }
    auto fixtures = Cases();
    std::array<std::vector<Trace>, 4> results;
    std::array<ErrorSamples, 4> oracle_errors;
    for (std::size_t profile = 0; profile < profiles.size(); ++profile) {
#ifdef __APPLE__
      std::string options = "-cl-std=CL1.2";
#else
      std::string options = "-cl-std=CL2.0";
#endif
      options += " -I\"" + std::string(GGEMS_E1_KERNEL_ROOT) + "\"";
      options += " -DGGEMS_RANDOM_ENGINE=3";
#ifdef GGEMS_DEBUG_MODE
      options += " -g -cl-opt-disable";
#endif
      options +=
        " -DGGEMS_SPHERE_FP32=" + std::string(profile == 1 ? "1" : "0");
      options += " -DGGEMS_ENABLE_FP64_NAVIGATION=" +
                 std::string(profile == 0 ? "1" : "0");
      constexpr std::array<std::string_view, 4> integer_flags{"0", "0", "64",
                                                              "32"};
      options +=
        " -DGGEMS_SPHERE_INTEGER=" + std::string(integer_flags[profile]);
      constexpr std::array<std::string_view, 4> integer_domains{
        "", "",
        "exact relative pm |q_i| and radius <=2^42; |stored D_i|<=4; nonzero "
        "quantized Q17 direction; uint128 discriminant and uint64 rational",
        "local |q_i|<=32767, radius1..32767 at2^24pm; |stored D_i|<=4; nonzero "
        "Q12 direction; int32 A/B,int64 C/discriminant"};
      Row(output, "profile",
          {{"device", Number(device_index)},
           {"name", Quote(profiles[profile])},
           {"spatial_quantum_pm",
            profile >= 2 ? Quote(Number(profile == 3 ? 1LL << 24 : 1LL))
                         : "null"},
           {"direction_fraction_bits",
            profile >= 2 ? Number(profile == 3 ? 12 : 17) : "null"},
           {"integer_rounding",
            profile >= 2
              ? Quote(
                  "input nearest, half away from zero; root floor square root; "
                  "endpoint nearest with final-coordinate half away from zero")
              : "null"},
           {"integer_domain",
            profile >= 2 ? Quote(integer_domains[profile]) : "null"}});
      cl::Program program(context.GetContextNative(), source, false, &error);
      Check(error, "create probe program");
      Row(output, "build_begin",
          {{"device", Number(device_index)},
           {"profile", Quote(profiles[profile])},
           {"requested_options", Quote(options)}});
      auto const status =
        program.build({device.GetDeviceNative()}, options.c_str());
      auto const effective = program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(
        device.GetDeviceNative(), &error);
      Check(error, "effective build options");
      auto const log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(
        device.GetDeviceNative(), &error);
      Check(error, "build log");
      Row(output, "build",
          {{"device", Number(device_index)},
           {"profile", Quote(profiles[profile])},
           {"requested_options", Quote(options)},
           {"effective_options", Quote(effective)},
           {"opencl_status", Number(status)},
           {"log", Quote(log)}});
      Check(status, "build probe");
      if (profile == 0) {
        CaptureCone(context, program, fixtures, output, device_index);
      }
      for (auto const &fixture : fixtures) {
        results[profile].push_back(Execute(context, program, fixture));
        EmitTrace(output, fixture, results[profile].back(), device_index,
                  profile, oracle_errors[profile]);
      }
    }
    constexpr std::array<std::string_view, 4> group_names{
      "sphere_identical_anchor", "sphere_propagated_anchor",
      "world_identical_anchor", "world_propagated_anchor"};
    for (std::size_t profile = 1; profile < profiles.size(); ++profile) {
      Summary summary;
      for (std::size_t index = 0; index < fixtures.size(); ++index) {
        Compare(output, fixtures[index], results[0][index],
                results[profile][index], device_index, profile, summary);
      }
      Row(output, "summary",
          {{"device", Number(device_index)},
           {"profile", Quote(profiles[profile])},
           {"reference", Quote(profiles[0])},
           {"rays", Number(summary.rays)},
           {"rays_with_category_disagreements",
            Number(summary.category_disagreements)},
           {"fp64_unresolved", Number(summary.fp64_unresolved)},
           {"candidate_unresolved", Number(summary.candidate_unresolved)},
           {"accuracy_acceptance",
            Quote("none; all numerical results are observations")}});
      for (std::size_t group = 0; group < summary.paired.size(); ++group) {
        auto const &samples = summary.paired[group];
        Distribution(output, device_index,
                     std::string(group_names[group]) +
                       "_absolute_paired_parameter_difference",
                     samples.parameters, samples.parameter_rays,
                     fixtures.size(), profile);
        Distribution(output, device_index,
                     std::string(group_names[group]) +
                       "_absolute_paired_endpoint_coordinate_difference",
                     samples.endpoints, samples.endpoint_rays, fixtures.size(),
                     profile);
        Distribution(output, device_index,
                     std::string(group_names[group]) +
                       "_absolute_paired_path_length_difference",
                     samples.paths, samples.path_rays, fixtures.size(),
                     profile);
      }
      std::cout << device.GetName() << ": " << profiles[profile]
                << " vs fp64: " << summary.rays << " identical-ray pairs; "
                << summary.category_disagreements
                << " categorical disagreements; UNRESOLVED="
                << summary.candidate_unresolved << '\n';
    }
    for (std::size_t profile = 0; profile < profiles.size(); ++profile) {
      std::size_t classified{};
      std::size_t unresolved{};
      for (auto const &trace : results[profile]) {
        bool known = trace[1] == 1;
        for (std::size_t attempt = 0; attempt < 3; ++attempt) {
          auto const offset = 8 + (16 * attempt);
          known |= trace[offset + 15] == 1 &&
                   (trace[offset + 2] == 1 || trace[offset + 2] == 2);
        }
        classified += known ? 1U : 0U;
        unresolved += trace[2] == 2 ? 1U : 0U;
      }
      Row(output, "profile_summary",
          {{"device", Number(device_index)},
           {"profile", Quote(profiles[profile])},
           {"rays", Number(fixtures.size())},
           {"classified_rays", Number(classified)},
           {"unclassified_rays", Number(fixtures.size() - classified)},
           {"unresolved_rays", Number(unresolved)}});
      auto const &samples = oracle_errors[profile];
      Distribution(output, device_index,
                   "absolute_approximate_oracle_sphere_parameter_difference",
                   samples.parameters, samples.parameter_rays, fixtures.size(),
                   profile);
      Distribution(
        output, device_index,
        "absolute_approximate_oracle_sphere_endpoint_coordinate_difference",
        samples.endpoints, samples.endpoint_rays, fixtures.size(), profile);
    }
  }
  Row(output, "completion",
      {{"selected_devices", Number(opencl.GetContext().size())},
       {"unsupported_devices", Number(unavailable)},
       {"status", Quote(unavailable != 0 ? "incomplete" : "observed")}});
  return unavailable != 0 ? 2 : 0;
}
} // namespace

auto main(int argc, char **argv) -> int {
  std::ofstream output;
  try {
    if (argc == 1 || (argc == 2 && std::string_view(argv[1]) == "--help")) {
      std::cout
        << "ggems_sphere_precision --device SELECTOR --output FILE.jsonl\n";
      return 0;
    }
    std::string selector;
    std::string path;
    for (int index = 1; index < argc; ++index) {
      std::string_view const key(argv[index]);
      if (++index == argc) {
        throw std::invalid_argument("missing argument value");
      }
      if (key == "--device") {
        selector = argv[index];
      } else if (key == "--output") {
        path = argv[index];
      } else {
        throw std::invalid_argument("unknown argument: " + std::string(key));
      }
    }
    if (selector.empty() || path.empty()) {
      throw std::invalid_argument("--device and --output are required");
    }
    output.exceptions(std::ios::failbit | std::ios::badbit);
    output.open(path, std::ios::out | std::ios::trunc);
    output.imbue(std::locale::classic());
    return Run(selector, output);
  } catch (std::exception const &exception) {
    std::cerr << "Sphere E2: " << exception.what() << '\n';
    if (output.is_open() && output.good()) {
      Row(output, "technical_error", {{"message", Quote(exception.what())}});
    }
    return 2;
  }
}
