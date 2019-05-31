//
//  input_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "input_range_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/input.hpp>

namespace py = pybind11;
using namespace blocksci;

void addInputRangeMethods(RangeClasses<Input> &classes) {
    addRangeMethods(classes);
}