//
//  witness_scripthash_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "witness_scripthash_range_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void addWitnessScriptHashRangeMethods(RangeClasses<script::WitnessScriptHash> &classes) {
	addRangeMethods(classes);
}