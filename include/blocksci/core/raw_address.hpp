//
//  raw_address.hpp
//  bitcoinapi
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef raw_address_hpp
#define raw_address_hpp

#include "core_fwd.hpp"
#include "address_types.hpp"

#include <blocksci/blocksci_export.h>

#include <functional>

namespace blocksci {
	class ScriptAccess;
	
    struct BLOCKSCI_EXPORT RawAddress {
        uint32_t scriptNum;
        AddressType::Enum type;
        
        RawAddress() {}
        RawAddress(uint32_t addressNum_, AddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
    };
    
    void BLOCKSCI_EXPORT visit(const RawAddress &address, const std::function<bool(const RawAddress &)> &visitFunc, const ScriptAccess &scripts);
} // namespace blocksci

#endif /* raw_address_hpp */
