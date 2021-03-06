//
//  raw_transaction.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef raw_transaction_hpp
#define raw_transaction_hpp

#include "inout.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/typedefs.hpp>

namespace blocksci {
    struct BLOCKSCI_EXPORT RawTransaction {
        uint32_t realSize;
        uint32_t baseSize;
        uint32_t locktime;
        uint16_t inputCount;
        uint16_t outputCount;
        uint16_t vpubCount;
        int64_t valueBalance;
        uint16_t nShieldedSpend;
        uint16_t nShieldedOutput;
        
        RawTransaction(uint32_t realSize_, uint32_t baseSize_, uint32_t locktime_, uint16_t inputCount_, uint16_t outputCount_, uint16_t vpubCount_, int64_t valueBalance_, uint16_t nShieldedSpend_, uint16_t nShieldedOutput_) : realSize(realSize_), baseSize(baseSize_), locktime(locktime_), inputCount(inputCount_), outputCount(outputCount_), vpubCount(vpubCount_), valueBalance(valueBalance_), nShieldedSpend(nShieldedSpend_), nShieldedOutput(nShieldedOutput_) {}
        
        RawTransaction(const RawTransaction &) = delete;
        RawTransaction(RawTransaction &&) = delete;
        RawTransaction &operator=(const RawTransaction &) = delete;
        RawTransaction &operator=(RawTransaction &&) = delete;
        
        Inout &getOutput(uint16_t outputNum) {
            return getInouts()[inputCount + outputNum];
        }
        
        Inout &getInput(uint16_t inputNum) {
            return getInouts()[inputNum];
        }
        
        const Inout &getOutput(uint16_t outputNum) const {
            return getInouts()[inputCount + outputNum];
        }
        
        const Inout &getInput(uint16_t inputNum) const {
            return getInouts()[inputNum];
        }

        const Inout *beginInputs() const {
            return &getInput(0);
        }

        const Inout *endInputs() const {
            return &getInput(0) + inputCount;
        }

        const Inout *beginOutputs() const {
            return &getOutput(0);
        }

        const Inout *endOutputs() const {
            return &getOutput(0) + outputCount;
        }
        
        size_t serializedSize() const {
            return sizeof(RawTransaction) + sizeof(Inout) * (inputCount + outputCount) + (vpubCount * sizeof(uint64_t) * 2);
        }
        
    private:
        const Inout *getInouts() const {
            return reinterpret_cast<const Inout *>(this + 1);
        }
        
        Inout *getInouts() {
            return reinterpret_cast<Inout *>(this + 1);
        }
    };
} // namespace blocksci

#endif /* raw_transaction_hpp */