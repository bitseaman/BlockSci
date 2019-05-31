//
//  scripthash_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "scripthash_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void init_scripthash(py::class_<script::ScriptHash> &cl) {
    cl
    .def("__repr__", &script::ScriptHash::toString)
    .def("__str__", &script::ScriptHash::toPrettyString)
    ;

    applyMethodsToSelf(cl, AddScriptHashBaseMethods<script::ScriptHash>{});
}

void init_witness_scripthash(py::class_<script::WitnessScriptHash> &cl) {
    
    cl
    .def("__repr__", &script::WitnessScriptHash::toString)
    .def("__str__", &script::WitnessScriptHash::toPrettyString)
    ;
    
    applyMethodsToSelf(cl, AddScriptHashBaseMethods<script::WitnessScriptHash>{});
}