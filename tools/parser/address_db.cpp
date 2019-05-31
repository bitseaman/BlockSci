//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_db.hpp"

#include <blocksci/core/address_info.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

using blocksci::Address;
using blocksci::RawAddress;
using blocksci::DedupAddress;
using blocksci::Transaction;
using blocksci::OutputPointer;
using blocksci::State;
using blocksci::DedupAddressType;
using blocksci::script::ScriptHash;

AddressDB::AddressDB(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "addressDB"), db(path, false) {
    outputCache.reserve(cacheSize);
    nestedCache.reserve(cacheSize);
}

AddressDB::~AddressDB() {
    clearNestedCache();
    clearOutputCache();
}

void AddressDB::processTx(const blocksci::RawTransaction *tx, uint32_t txNum, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    std::function<bool(const RawAddress &)> visitFunc = [&](const RawAddress &a) {
        if (dedupType(a.type) == DedupAddressType::SCRIPTHASH) {
            auto scriptHash = scripts.getScriptData<DedupAddressType::SCRIPTHASH>(a.scriptNum);
            if (scriptHash->txFirstSeen == txNum) {
                addAddressNested(scriptHash->wrappedAddress, DedupAddress{a.scriptNum, DedupAddressType::SCRIPTHASH});
                return true;
            } else {
                return false;
            }
        }
        return false;
    };
    auto inputs = ranges::make_iterator_range(tx->beginInputs(), tx->endInputs());
    for (auto &input : inputs) {
        visit(RawAddress{input.getAddressNum(), input.getType()}, visitFunc, scripts);
    }
    
    for (uint16_t i = 0; i < tx->outputCount; i++) {
        auto &output = tx->getOutput(i);
        auto pointer = OutputPointer{txNum, i};
        addAddressOutput(blocksci::RawAddress{output.getAddressNum(), output.getType()}, pointer);
    }
}

void AddressDB::addAddressNested(const blocksci::RawAddress &childAddress, const blocksci::DedupAddress &parentAddress) {
    nestedCache.emplace_back(childAddress, parentAddress);
    if (nestedCache.size() >= cacheSize) {
        clearNestedCache();
    }
}

void AddressDB::clearNestedCache() {
    db.addNestedAddresses(std::move(nestedCache));
    nestedCache.clear();
}

void AddressDB::addAddressOutput(const blocksci::RawAddress &address, const blocksci::OutputPointer &pointer) {
    outputCache.emplace_back(address, pointer);
    if (outputCache.size() >= cacheSize) {
        clearOutputCache();
    }
}

void AddressDB::clearOutputCache() {
    db.addOutputAddresses(std::move(outputCache));
    outputCache.clear();
}
