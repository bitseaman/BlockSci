//
//  address_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "address_range_py.hpp"
#include "blocksci_range.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

namespace py = pybind11;
using namespace blocksci;

void addAddressRangeMethods(RangeClasses<AnyScript> &classes) {
    addRangeMethods(classes);
}