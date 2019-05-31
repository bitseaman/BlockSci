//
//  equiv_address_range_methods.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "equiv_address_range_py.hpp"
#include "range_apply_py.hpp"
#include "equiv_address_py.hpp"
#include "blocksci_range.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>

namespace py = pybind11;
using namespace blocksci;

void applyMethodsToEquivAddressRange(RangeClasses<EquivAddress> &classes) {
    applyMethodsToRange(classes, AddEquivAddressMethods{});
}
