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
 * \brief Defines Python bindings for GGEMS random configuration.
 *
 * Exposes random-engine selection, seed configuration, verbosity, and representation through pybind11.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <string>
#include <memory>
#include <cstdint>
#include <format>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

/// \endcond
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"

namespace py = pybind11;

/*!
 * \brief Registers GGEMS random-engine bindings in a Python module.
 *
 * Exposes GGEMSRandomEngine and GGEMSRandom, including engine selection, seed configuration, verbose output, and a concise representation.
 *
 * \param[in,out] module Python module receiving the random bindings.
 */
void BindRandom(py::module_ &module) {
  using ggems::core::random::GGEMSRandom;
  using ggems::core::random::GGEMSRandomEngine;

  py::enum_<GGEMSRandomEngine>(
      module, "GGEMSRandomEngine",
      R"doc(Random-number engines available to GGEMS.

This enum exposes the engine names used by the GGEMS random subsystem.
GGEMSRandom.set_engine() currently selects an engine by string name.
)doc")
      .value("JKISS", GGEMSRandomEngine::JKISS,
             "JKISS random-number engine.")
      .value("PCG32", GGEMSRandomEngine::PCG32,
             "PCG32 random-number engine.")
      .value("Philox", GGEMSRandomEngine::Philox,
             "Philox random-number engine used by default.");

  py::class_<GGEMSRandom, std::shared_ptr<GGEMSRandom>>(
      module, "GGEMSRandom",
      R"doc(Configure the random-number streams used by GGEMS simulations.

A new configuration uses the Philox engine with seed 77777.

This object configures GGEMS random streams; it is not a Python random-number
generator and does not directly expose sample-generation methods.

set_engine() and set_seed() return the same object, so configuration calls can
be chained.

Example:
    rng = ggems.GGEMSRandom().set_engine("Philox").set_seed(12345)
    rng.verbose()
)doc")
      .def(py::init<>(),
           R"doc(Create a random configuration using Philox and seed 77777.)doc")

      .def(
          "set_engine",
          [](GGEMSRandom &self, std::string const &engine) -> GGEMSRandom & {
            return self.SetEngine(engine);
          },
          R"doc(Select the GGEMS random engine by name.

Accepted names are "JKISS" (or "KISS"), "PCG32" (or "PCG"), and "Philox".
Matching is case-insensitive, and spaces, hyphens, and underscores are ignored.

Parameters:
    engine: Engine name.

Returns:
    This GGEMSRandom object, allowing chained configuration.

Example:
    rng.set_engine("pcg32").set_seed(42)
)doc",
          py::arg("engine"), py::return_value_policy::reference_internal)

      .def(
          "set_seed",
          [](GGEMSRandom &self, std::uint64_t seed) -> GGEMSRandom & {
            return self.SetSeed(seed);
          },
          R"doc(Set the unsigned 64-bit seed used to initialize GGEMS streams.

Using a fixed engine and seed gives deterministic random-stream initialization.
The accepted Python integer range is 0 through 2**64 - 1.

Parameters:
    seed: Unsigned 64-bit seed.

Returns:
    This GGEMSRandom object, allowing chained configuration.
)doc",
          py::arg("seed"), py::return_value_policy::reference_internal)

      .def(
          "verbose", &GGEMSRandom::Verbose,
          R"doc(Print the current GGEMS random configuration through the GGEMS logger.

The report includes the selected engine, seed, state size, OpenCL engine ID,
build definition, and generic kernel random APIs.
)doc")

      .def("__repr__", [](GGEMSRandom const &random) -> std::string {
        return std::format("<GGEMSRandom engine='{}' seed={}>",
                           random.GetEngineName(), random.GetSeed());
      });
}
