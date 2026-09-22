#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutDescription.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace py = pybind11;

namespace {

namespace materials = ggems::core::materials;
namespace processes = ggems::core::processes;

// =============================================================================
// =============================================================================

auto MakeLength(std::optional<double> value, std::string const &unit)
  -> std::optional<ggems::units::Length> {
  if (!value.has_value()) {
    return std::nullopt;
  }

  return ggems::python::detail::MakeQuantityOrThrow<ggems::units::Length>(
    *value, unit,
    {
      .quantity_name = "Production-Cut length",
      .unsupported_unit_subject = "GGEMS length",
    });
}

} // namespace

// =============================================================================
// =============================================================================

auto BindCuts(py::module_ &module) -> void {
  // === === === ===
  py::enum_<processes::GGEMSProductionCutChannel>(module,
                                                  "ProductionCutChannel")
    .value("Gamma", processes::GGEMSProductionCutChannel::Gamma)
    .value("Electron", processes::GGEMSProductionCutChannel::Electron)
    .value("Positron", processes::GGEMSProductionCutChannel::Positron)
    .value("Proton", processes::GGEMSProductionCutChannel::Proton);

  // === === === ===

  py::enum_<processes::GGEMSProductionCutScope>(module, "ProductionCutScope")
    .value("Global", processes::GGEMSProductionCutScope::Global)
    .value("Material", processes::GGEMSProductionCutScope::Material)
    .value("Volume", processes::GGEMSProductionCutScope::Volume);

  // === === === ===
  py::class_<processes::GGEMSProductionCutLengths>(module, "Lengths")
    .def(py::init(
           [](std::optional<double> gamma, std::optional<double> electron,
              std::optional<double> positron, std::optional<double> proton,
              std::string const &unit) -> processes::GGEMSProductionCutLengths {
             return {
               .gamma = MakeLength(gamma, unit),
               .electron = MakeLength(electron, unit),
               .positron = MakeLength(positron, unit),
               .proton = MakeLength(proton, unit),
             };
           }),
         py::kw_only(), py::arg("gamma") = py::none(),
         py::arg("electron") = py::none(), py::arg("positron") = py::none(),
         py::arg("proton") = py::none(), py::arg("unit") = "mm");

  // === === === ===
  py::class_<processes::GGEMSProductionCutPolicy>(module, "Policy")
    .def(
      py::init(
        [](processes::GGEMSProductionCutLengths const &global_lengths,
           std::map<std::uint32_t, processes::GGEMSProductionCutLengths> const
             &material_overrides) -> processes::GGEMSProductionCutPolicy {
          processes::GGEMSProductionCutPolicy policy{
            .global = global_lengths,
            .materials = {},
          };

          for (auto const &[material_index, lengths] : material_overrides) {
            policy.materials.push_back({
              .material_index = material_index,
              .lengths = lengths,
            });
          }

          return policy;
        }),
      py::arg("global_lengths"),
      py::arg("material_overrides") =
        std::map<std::uint32_t, processes::GGEMSProductionCutLengths>{});

  // === === === ===
  py::class_<processes::GGEMSProductionCutContext>(module, "Context")
    .def(py::init([](std::uint32_t material_index,
                     std::optional<processes::GGEMSProductionCutLengths> const
                       &volume) -> processes::GGEMSProductionCutContext {
           return {
             .material_index = material_index,
             .volume = volume.value_or(processes::GGEMSProductionCutLengths{}),
           };
         }),
         py::arg("material_index"), py::arg("volume") = py::none());

  // === === === ===
  py::class_<processes::GGEMSProductionCutChannelInspection>(
    module, "ProductionCutChannelInspection")
    .def_readonly("channel",
                  &processes::GGEMSProductionCutChannelInspection::channel)

    .def_property_readonly(
      "effective_length_pm",
      [](processes::GGEMSProductionCutChannelInspection const &channel)
        -> std::uint64_t { return channel.effective_length.value; })

    .def_readonly(
      "winning_scope",
      &processes::GGEMSProductionCutChannelInspection::winning_scope)

    .def_property_readonly(
      "production_threshold_micro_eV",
      [](processes::GGEMSProductionCutChannelInspection const &channel)
        -> std::uint64_t { return channel.production_threshold.value; });

  // === === === ===
  py::class_<processes::GGEMSProductionCutContextInspection>(
    module, "ProductionCutContextInspection")
    .def_readonly(
      "context_index",
      &processes::GGEMSProductionCutContextInspection::context_index)

    .def_readonly(
      "authoring_material_index",
      &processes::GGEMSProductionCutContextInspection::authoring_material_index)

    .def_readonly(
      "snapshot_material_id",
      &processes::GGEMSProductionCutContextInspection::snapshot_material_id)

    .def_readonly(
      "snapshot_couple_id",
      &processes::GGEMSProductionCutContextInspection::snapshot_couple_id)

    .def_readonly("channels",
                  &processes::GGEMSProductionCutContextInspection::channels);

  // === === === ===
  py::class_<processes::GGEMSMaterialCutCouplePackage>(
    module, "MaterialCutCouplePackage")
    .def(py::init([](materials::GGEMSEMMaterialPackage const &materials,
                     processes::GGEMSProductionCutPolicy const &policy,
                     std::vector<processes::GGEMSProductionCutContext> const
                       &contexts) -> processes::GGEMSMaterialCutCouplePackage {
           return processes::GGEMSMaterialCutCouplePackage{materials, policy,
                                                           contexts};
         }),
         py::arg("materials"), py::arg("policy"), py::arg("contexts"))

    .def_property_readonly(
      "couple_count",
      [](processes::GGEMSMaterialCutCouplePackage const &package)
        -> std::size_t { return package.GetCouples().size(); })

    .def_property_readonly(
      "context_couple_ids",
      [](processes::GGEMSMaterialCutCouplePackage const &package)
        -> std::vector<std::uint32_t> {
        auto const ids = package.GetContextCoupleIds();
        return {ids.begin(), ids.end()};
      })

    .def(
      "inspect",
      [](processes::GGEMSMaterialCutCouplePackage const &package,
         std::size_t context_index)
        -> processes::GGEMSProductionCutContextInspection {
        return processes::InspectProductionCutContext(package, context_index);
      },
      py::arg("context_index"))

    .def(
      "describe",
      [](processes::GGEMSMaterialCutCouplePackage const &package,
         std::size_t context_index) -> std::string {
        return processes::DescribeProductionCutContext(package, context_index);
      },
      py::arg("context_index"))

    .def(
      "verbose",
      [](processes::GGEMSMaterialCutCouplePackage const &package,
         std::size_t context_index) -> void {
        processes::VerboseProductionCutContext(package, context_index);
      },
      py::arg("context_index"));
}
