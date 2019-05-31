//
//  multisig_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_multisig_range_py_h
#define blocksci_multisig_range_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/multisig_script.hpp>

void addMultisigRangeMethods(RangeClasses<blocksci::script::Multisig> &classes);
void applyMethodsToMultisigRange(RangeClasses<blocksci::script::Multisig> &classes);


#endif /* blocksci_multisig_range_py_h */
