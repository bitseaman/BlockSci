//
//  block_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "block_range_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/block.hpp>

namespace py = pybind11;
using namespace blocksci;

void addBlockRangeMethods(RangeClasses<Block> &classes) {
    addRangeMethods(classes);
}