//
//  equiv_address_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "equiv_address_range_py.hpp"
#include "blocksci_range.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

namespace py = pybind11;
using namespace blocksci;

void addEquivAddressRangeMethods(RangeClasses<EquivAddress> &classes) {
    addRangeMethods(classes);
}