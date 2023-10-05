/*
 * Copyright 2022 Google LLC.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "pybind11_abseil/status_casters.h"
#include "pybind11_protobuf/native_proto_caster.h"
#include "yggdrasil_decision_forests/model/abstract_model.h"
#include "yggdrasil_decision_forests/model/gradient_boosted_trees/gradient_boosted_trees.h"
#include "yggdrasil_decision_forests/model/model_library.h"
#include "yggdrasil_decision_forests/model/random_forest/random_forest.h"
#include "ydf/model/model_wrapper.h"

namespace py = ::pybind11;

namespace yggdrasil_decision_forests::port::python {
namespace {

absl::StatusOr<std::unique_ptr<GenericCCModel>> LoadModel(
    const std::string& directory,
    const std::optional<std::string>& file_prefix) {
  std::unique_ptr<model::AbstractModel> model_ptr;
  RETURN_IF_ERROR(model::LoadModel(directory, &model_ptr, {file_prefix}));
  return CreateCCModel(std::move(model_ptr));
}
}  // namespace

void init_model(py::module_& m) {
  m.def("LoadModel", LoadModel, py::arg("directory"), py::arg("file_prefix"));
  py::class_<GenericCCModel>(m, "GenericCCModel")
      .def("__repr__",
           [](const GenericCCModel& a) {
             return absl::Substitute("<model_cc.GenericCCModel of type $0.",
                                     a.name());
           })
      .def("Predict", &GenericCCModel::Predict, py::arg("dataset"))
      .def("Evaluate", &GenericCCModel::Evaluate, py::arg("dataset"),
           py::arg("options"))
      .def("save", &GenericCCModel::Save, py::arg("directory"),
           py::arg("file_prefix"))
      .def("name", &GenericCCModel::name)
      .def("task", &GenericCCModel::task)
      .def("data_spec", &GenericCCModel::data_spec);

  py::class_<DecisionForestCCModel,
             /*parent class*/ GenericCCModel>(m, "DecisionForestCCModel")
      .def("__repr__",
           [](const DecisionForestCCModel& a) {
             return absl::Substitute(
                 "<model_cc.DecisionForestCCModel of type $0.", a.name());
           })
      .def("num_trees", &DecisionForestCCModel::num_trees);

  py::class_<RandomForestCCModel,
             /*parent class*/ DecisionForestCCModel>(m, "RandomForestCCModel")
      .def("__repr__",
           [](const GenericCCModel& a) {
             return absl::Substitute(
                 "<model_cc.RandomForestCCModel of type $0.", a.name());
           })
      .def_property_readonly_static(
          "kRegisteredName", [](py::object /* self */) {
            return model::random_forest::RandomForestModel::kRegisteredName;
          });

  py::class_<GradientBoostedTreesCCModel,
             /*parent class*/ DecisionForestCCModel>(
      m, "GradientBoostedTreesCCModel")
      .def("__repr__",
           [](const GenericCCModel& a) {
             return absl::Substitute(
                 "<model_cc.GradientBoostedTreesCCModel of type $0.", a.name());
           })
      .def("validation_loss", &GradientBoostedTreesCCModel::validation_loss)
      .def_property_readonly_static(
          "kRegisteredName", [](py::object /* self */) {
            return model::gradient_boosted_trees::GradientBoostedTreesModel::
                kRegisteredName;
          });
}

}  // namespace yggdrasil_decision_forests::port::python
