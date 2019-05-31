//
//  pubkey_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "pubkey_range_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void addPubkeyRangeMethods(RangeClasses<script::Pubkey> &classes) {
	addRangeMethods(classes);
}