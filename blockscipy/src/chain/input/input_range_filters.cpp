//
//  input_range_filters.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "input_range_py.hpp"
#include "caster_py.hpp"
#include "range_filter_apply_py.hpp"

#include <blocksci/chain/algorithms.hpp>

namespace py = pybind11;
using namespace blocksci;

template <typename Range>
struct AddInputRangeMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        py::options options;
        options.disable_function_signatures();

        func(method_tag, "sent_before_height",  [](Range &range, BlockHeight height) {
            return inputsCreatedBeforeHeight(range, height);
        }, "Filter the inputs to include only inputs which spent an output created before the given height", py::arg("height"));
        func(method_tag, "sent_after_height",  [](Range &range, BlockHeight height) {
            return inputsCreatedAfterHeight(range, height);
        }, "Filter the inputs to include only inputs which spent an output created after the given height", py::arg("height"));
        func(method_tag, "with_age_less_than",  [](Range &range, BlockHeight height) {
            return inputsCreatedWithinRelativeHeight(range, height);
        }, "Filter the inputs to include only inputs with age less than the given value", py::arg("age"));
        func(method_tag, "with_age_greater_than",  [](Range &range, BlockHeight height) {
            return inputsCreatedOutsideRelativeHeight(range, height);
        }, "Filter the inputs to include only inputs with age more than the given value", py::arg("age"));
        func(method_tag, "with_address_type", [](Range &range, AddressType::Enum type) {
            return inputsOfType(range, type);
        }, "Filter the inputs to include only inputs that came from an address with the given type", py::arg("type"));
    }
};

void applyRangeFiltersToInputRange(RangeClasses<Input> &classes) {
    applyRangeFiltersToRange<AddInputRangeMethods>(classes);
}
