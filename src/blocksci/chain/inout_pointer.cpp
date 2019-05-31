//
//  inout_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "inout_pointer.hpp"
#include "util/hash.hpp"
#include "chain_access.hpp"

#include <sstream>

namespace blocksci {
    std::string InputPointer::toString() const {
        std::stringstream ss;
        ss << "InputPointer(tx_index=" << txNum << ", input_index=" << inoutNum << ")";
        return ss.str();
    }
    
    std::string OutputPointer::toString() const {
        std::stringstream ss;
        ss << "OutputPointer(tx_index_from=" << txNum << ", output_index_from=" << inoutNum << ")";
        return ss.str();
    }
    
    bool InputPointer::isValid(const ChainAccess &access) {
        return inoutNum < access.getTx(txNum)->inputCount;
    }
    
    bool OutputPointer::isValid(const ChainAccess &access) {
        return inoutNum < access.getTx(txNum)->outputCount;
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::InputPointer &pointer) {
    os << pointer.toString();
    return os;
}

std::ostream &operator<<(std::ostream &os, const blocksci::OutputPointer &pointer) {
    os << pointer.toString();
    return os;
}

namespace std
{
    size_t hash<blocksci::InputPointer>::operator()(const blocksci::InputPointer &pointer) const {
        std::size_t seed = 41352363;
        hash_combine(seed, pointer.txNum);
        hash_combine(seed, pointer.inoutNum);
        return seed;
    }
    
    size_t hash<blocksci::OutputPointer>::operator()(const blocksci::OutputPointer &pointer) const {
        std::size_t seed = 41352363;
        hash_combine(seed, pointer.txNum);
        hash_combine(seed, pointer.inoutNum);
        return seed;
    }
}
