//
//  scripthash_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "scripthash_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"
#include "bitcoin_base58.hpp"

namespace blocksci {
    using namespace script;
    
    ScriptHash::ScriptAddress(const ScriptHashData *rawData) : address(rawData->address), wrappedAddress(rawData->wrappedAddress) {}
    
    ScriptHash::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : ScriptHash(access.getScriptData<AddressType::Enum::SCRIPTHASH>(addressNum)) {}
    
    ScriptHash::ScriptAddress(const uint160 &address_) : address(address_), wrappedAddress() {}
    
    bool ScriptHash::operator==(const Script &other) {
        auto otherA = dynamic_cast<const ScriptHash *>(&other);
        return otherA && otherA->address == address;
    }
    
    std::string ScriptHash::addressString(const DataConfiguration &config) const {
        return CBitcoinAddress(address, AddressType::Enum::SCRIPTHASH, config).ToString();
    }
    
    std::string ScriptHash::toString(const DataConfiguration &config) const {
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString(config);
        ss << ")";
        return ss.str();
    }
    
    std::string ScriptHash::toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const {
        auto wrapped = wrappedScript(access);
        std::stringstream ss;
        ss << "P2SHAddress(";
        ss << "address=" << addressString(config);
        ss << ", wrappedAddress=";
        if (wrappedAddress.addressNum > 0) {
            ss << wrapped->toPrettyString(config, access);
        } else {
            ss << "unknown";
        }
        
        ss << ")";
        return ss.str();
    }
}
