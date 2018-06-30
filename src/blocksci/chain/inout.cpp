//
//  inout.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/28/17.
//
//

#include "inout.hpp"
#include "address/address.hpp"

#include "util/hash.hpp"

#include <sstream>

namespace blocksci {
    
    Inout::Inout(uint32_t linkedTxNum_, const Address &address, uint64_t value) : linkedTxNum(linkedTxNum_), toAddressNum(address.scriptNum), other(0) {
        setValue(value);
        setType(address.type);
    }
    
    Address Inout::getAddress() const {
        return Address(toAddressNum, getType());
    }
    
    bool Inout::operator==(const Inout& otherInout) const {
        return linkedTxNum == otherInout.linkedTxNum && toAddressNum == otherInout.toAddressNum && other == otherInout.other;
    }
}

namespace std
{
    size_t hash<blocksci::Inout>::operator()(const blocksci::Inout &inout) const {
        std::size_t seed = 65246342;
        hash_combine(seed, inout.linkedTxNum);
        hash_combine(seed, inout.toAddressNum);
        hash_combine(seed, inout.other);
        return seed;
    }
}
