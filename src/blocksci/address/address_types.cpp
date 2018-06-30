//
//  address_types.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 2/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_types.hpp"
#include "address_info.hpp"
#include <iostream>

namespace blocksci {
    
}

std::ostream &operator<<(std::ostream &os, blocksci::AddressType::Enum const &type) {
    os << addressName(type);
    return os;
}
