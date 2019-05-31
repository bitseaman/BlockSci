//
//  script.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "util/util.hpp"
#include "script.hpp"
#include "script_data.hpp"
#include "script_info.hpp"

#include "script_variant.hpp"

#include "address/address.hpp"
#include "address/address_info.hpp"
#include "index/address_index.hpp"

#include "chain/transaction.hpp"
#include "chain/output.hpp"

#include <iostream>


namespace blocksci {
    
    Script::Script(const Address &address) : Script(address.scriptNum, scriptType(address.type)) {}
    
    BaseScript::BaseScript(uint32_t scriptNum_, ScriptType::Enum type_, const ScriptDataBase &data, const ScriptAccess &scripts_) : Script(scriptNum_, type_), access(&scripts_), firstTxIndex(data.txFirstSeen), txRevealed(data.txFirstSpent) {}
    
    BaseScript::BaseScript(const Address &address, const ScriptDataBase &data, const ScriptAccess &scripts) : BaseScript(address.scriptNum, scriptType(address.type), data, scripts) {}
    
    
    Transaction BaseScript::getFirstTransaction(const ChainAccess &chain) const {
        return Transaction(firstTxIndex, chain);
    }
    
    ranges::optional<Transaction> BaseScript::getTransactionRevealed(const ChainAccess &chain) const {
        if (txRevealed != 0) {
            return Transaction(txRevealed, chain);
        } else {
            return ranges::nullopt;
        }
    }
    
    std::string Script::toString() const {
        if (scriptNum == 0) {
            return "InvalidScript()";
        } else {
            std::stringstream ss;
            ss << "Script(";
            ss << "scriptNum=" << scriptNum;
            ss << ", type=" << scriptName(type);
            ss << ")";
            return ss.str();
        }
    }
    
    AnyScript Script::getScript(const ScriptAccess &scripts) const {
        return AnyScript(*this, scripts);
    }
    
    std::vector<Output> Script::getOutputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputs(*this, chain);
    }
    
    std::vector<Input> Script::getInputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputs(*this, chain);
    }
    
    std::vector<Transaction> Script::getTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getTransactions(*this, chain);
    }
    
    std::vector<Transaction> Script::getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputTransactions(*this, chain);
    }
    
    std::vector<Transaction> Script::getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputTransactions(*this, chain);
    }

}
