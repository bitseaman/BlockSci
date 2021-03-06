//
//  tx_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tx_py_h
#define tx_py_h

#include "method_tags.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/heuristics/change_address.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

void init_tx(pybind11::class_<blocksci::Transaction> &cl);

struct AddTransactionMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;

        func(property_tag, "ins", &Transaction::inputs, "A list of the inputs of the transaction"); // same as below
        func(property_tag, "inputs", &Transaction::inputs, "A list of the inputs of the transaction"); // same as above
        func(property_tag, "outs", &Transaction::outputs, "A list of the outputs of the transaction"); // same as below
        func(property_tag, "outputs", &Transaction::outputs, "A list of the outputs of the transaction"); // same as above
        func(property_tag, "vpub_old", &Transaction::vpubolds, "A list of the vpubolds of the transaction"); // same as above
        func(property_tag, "vpub_new", &Transaction::vpubnews, "A list of the vpubnews of the transaction"); // same as above
        func(property_tag, "vpub_count", &Transaction::vpubCount, "The number of joinsplits in the transaction"); // same as above
        func(property_tag, "value_balance", &Transaction::valueBalance, "The valueBalance of the transaction"); // same as above
        func(property_tag, "sspend_count", &Transaction::shieldedSpendCount, "The number of shielded spends in the transaction"); // same as above
        func(property_tag, "soutput_count", &Transaction::shieldedOutputCount, "The number of shielded outputs in the transaction"); // same as above
        func(property_tag, "output_count", &Transaction::outputCount, "The number of outputs this transaction has");
        func(property_tag, "input_count", &Transaction::inputCount, "The number of inputs this transaction has");
        func(property_tag, "size_bytes", &Transaction::totalSize, "The size of this transaction in bytes");
        func(property_tag, "base_size", &Transaction::baseSize, "The size of the non-segwit data in bytes");
        func(property_tag, "total_size", &Transaction::totalSize, "The size all transaction data in bytes");
        func(property_tag, "virtual_size", &Transaction::virtualSize, "The weight of the transaction divided by 4");
        func(property_tag, "weight", &Transaction::weight, "Three times the base size plus the total size");
        func(property_tag, "locktime", &Transaction::locktime, "The locktime of this transasction");
        func(property_tag, "block_height", [](const Transaction &tx) { return tx.blockHeight; }, "The height of the block that this transaction was in");
        func(property_tag, "block_time", [](const Transaction &tx) -> std::chrono::system_clock::time_point {
            return tx.block().getTime();
        }, "The time that the block containing this transaction arrived");
        func(property_tag, "observed_in_mempool", &Transaction::observedInMempool, "Returns whether this transaction was seen in the mempool by the recorder");
        func(property_tag, "time_seen", &Transaction::getTimeSeen, "If recorded by the mempool recorder, the time that this transaction was first seen by your node");
        func(property_tag, "block", &Transaction::block, "The block that this transaction was in");
        func(property_tag, "index", [](const Transaction &tx) { return tx.txNum; }, "The internal index of this transaction");
        func(property_tag, "hash", &Transaction::getHash, "The 256-bit hash of this transaction");
        func(property_tag, "input_value", totalInputValue<Transaction &>, "The sum of the value of all of the inputs");
        func(property_tag, "output_value", totalOutputValue<Transaction &>, "The sum of the value of all of the outputs");
        func(property_tag, "fee", [](const Transaction &tx) -> int64_t {
            return fee(tx);
        }, "The fee paid by this transaction");
        func(property_tag, "sum_vpubold", [](const Transaction &tx) -> int64_t {
            return totalVpubold(tx);
        }, "The sum of vpub_old values");
        func(property_tag, "sum_vpubnew", [](const Transaction &tx) -> int64_t {
            return totalVpubnew(tx);
        }, "The sum of vpub_new values");
        func(method_tag, "fee_per_byte", [](const Transaction &tx, const std::string &sizeMeasure) -> int64_t {
            auto txFee = fee(tx);
            if (sizeMeasure == "total") {
                return txFee / tx.totalSize();
            } else if (sizeMeasure == "base") {
                return txFee / tx.baseSize();
            } else if(sizeMeasure == "weight") {
                return txFee / tx.weight();
            } else if(sizeMeasure == "virtual") {
                return txFee / tx.virtualSize();
            } else {
                throw std::invalid_argument{"Size measure must be one of total, base, weight, or virtual"};
            }
        }, "The ratio of fee paid to size of this transaction. By default this uses veritual size, but passing total, base, weight, or virtual let's you choose what measure of size you want", pybind11::arg("size_measure") = "virtual");
        func(property_tag, "op_return", [](const Transaction &tx) -> ranges::optional<Output> {
            return getOpReturn(tx);
        }, "If this transaction included a null data address, return its output. Otherwise return None");
        func(method_tag, "includes_output_of_type", includesOutputOfType, "Check whether the given transaction includes an output of the given address type", pybind11::arg("address_type"));
        func(property_tag, "is_coinbase", &Transaction::isCoinbase, "Return's true if this transaction is a Coinbase transaction");
        func(property_tag, "is_shielded", &Transaction::isShielded, "Return's true if this transaction is a shielded transaction");
        func(property_tag, "is_saplingshielded", &Transaction::isSaplingShielded, "Return's true if this transaction is a shielded transaction");
        func(property_tag, "is_sproutshielded", &Transaction::isSproutShielded, "Return's true if this transaction is a shielded transaction");
        func(property_tag, "change_output", [](const Transaction &tx) -> ranges::optional<Output> {
            return heuristics::uniqueChangeByLegacyHeuristic(tx);
        }, "If the change address in this transaction can be determined via the fresh address criteria, return it. Otherwise return None.");
    }
};


#endif /* tx_py_h */