//
//  main.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 12/30/15.
//  Copyright © 2015 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "config.hpp"
#include "parser_configuration.hpp"
#include "utxo_state.hpp"
#include "address_state.hpp"
#include "chain_index.hpp"
#include "preproccessed_block.hpp"
#include "block_processor.hpp"
#include "address_db.hpp"
#include "parser_index_creator.hpp"
#include "hash_index_creator.hpp"
#include "block_replayer.hpp"
#include "address_writer.hpp"
#include "utxo_address_state.hpp"

#include <blocksci/util/state.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <unordered_set>
#include <future>
#include <iostream>
#include <iomanip>
#include <cassert>

std::vector<char> HexToBytes(const std::string& hex);
uint32_t getStartingTxCount(const blocksci::DataConfiguration &config);


blocksci::State rollbackState(const ParserConfigurationBase &config, blocksci::BlockHeight firstDeletedBlock, uint32_t firstDeletedTxNum) {
    blocksci::State state{blocksci::ChainAccess{config, false, blocksci::BlockHeight{0}}, blocksci::ScriptAccess{config}};
    state.blockCount = static_cast<uint32_t>(static_cast<int>(firstDeletedBlock));
    state.txCount = firstDeletedTxNum;
    
    blocksci::IndexedFileMapper<blocksci::AccessMode::readwrite, blocksci::RawTransaction> txFile{config.txFilePath()};
    blocksci::FixedSizeFileMapper<blocksci::uint256, blocksci::AccessMode::readwrite> txHashesFile{config.txHashesFilePath()};
    blocksci::ScriptAccess scripts(config);
    
    UTXOState utxoState;
    UTXOAddressState utxoAddressState;
    UTXOScriptState utxoScriptState;
    
    utxoAddressState.unserialize(config.utxoAddressStatePath());
    utxoState.unserialize(config.utxoCacheFile().native());
    utxoScriptState.unserialize(config.utxoScriptStatePath().native());
    
    uint32_t totalTxCount = static_cast<uint32_t>(txFile.size());
    for (uint32_t txNum = totalTxCount - 1; txNum >= firstDeletedTxNum; txNum--) {
        auto tx = txFile.getData(txNum);
        auto hash = txHashesFile.getData(txNum);
        for (uint16_t i = 0; i < tx->outputCount; i++) {
            auto &output = tx->getOutput(i);
            if (output.getAddress().getScript(scripts).firstTxIndex() == txNum) {
                auto &prevValue = state.scriptCounts[static_cast<size_t>(scriptType(output.getType()))];
                auto addressNum = output.getAddress().scriptNum;
                if (addressNum < prevValue) {
                    prevValue = addressNum;
                }
            }
            if (isSpendable(scriptType(output.getType()))) {
                utxoState.erase({*hash, i});
                utxoAddressState.spendOutput({txNum, i}, output.getType());
                utxoScriptState.erase({txNum, i});
            }
        }
        
        uint32_t inputsAdded = 0;
        for (uint16_t i = 0; i < tx->inputCount; i++) {
            auto &input = tx->getInput(i);
            auto spentTxNum = input.linkedTxNum;
            auto spentTx = txFile.getData(spentTxNum);
            auto spentHash = txHashesFile.getData(spentTxNum);
            for (uint16_t j = 0; j < spentTx->outputCount; j++) {
                auto &output = spentTx->getOutput(j);
                if (output.linkedTxNum == txNum) {
                    output.linkedTxNum = 0;
                    UTXO utxo(output.getValue(), spentTxNum, output.getType());
                    utxoState.add({*spentHash, j}, utxo);
                    utxoAddressState.addOutput({output.getAddress().getScript(scripts), output.getType()}, {spentTxNum, j});
                    utxoScriptState.add({spentTxNum, j}, output.toAddressNum);
                    inputsAdded++;
                }
            }
        }
        assert(inputsAdded == tx->inputCount);
    }
    
    utxoAddressState.serialize(config.utxoAddressStatePath());
    utxoState.serialize(config.utxoCacheFile().native());
    utxoScriptState.serialize(config.utxoScriptStatePath().native());
    
    return state;
}

void rollbackTransactions(blocksci::BlockHeight blockKeepCount, const ParserConfigurationBase &config) {
    using namespace blocksci;
    
    constexpr auto readwrite = blocksci::AccessMode::readwrite;
    blocksci::FixedSizeFileMapper<blocksci::RawBlock, readwrite> blockFile(config.blockFilePath());
    
    auto blockKeepSize = static_cast<size_t>(static_cast<int>(blockKeepCount));
    if (blockFile.size() > blockKeepSize) {
        
        auto firstDeletedBlock = blockFile.getData(blockKeepSize);
        auto firstDeletedTxNum = firstDeletedBlock->firstTxIndex;
        
        auto blocksciState = rollbackState(config, blockKeepCount, firstDeletedTxNum);
        
        blocksci::IndexedFileMapper<readwrite, blocksci::RawTransaction>(config.txFilePath()).truncate(firstDeletedTxNum);
        blocksci::FixedSizeFileMapper<blocksci::uint256, readwrite>(config.txHashesFilePath()).truncate(firstDeletedTxNum);
        blocksci::IndexedFileMapper<readwrite, uint32_t>(config.sequenceFilePath()).truncate(firstDeletedTxNum);
        blocksci::SimpleFileMapper<readwrite>(config.blockCoinbaseFilePath()).truncate(firstDeletedBlock->coinbaseOffset);
        blockFile.truncate(blockKeepSize);
        
        AddressState(config.addressPath()).rollback(blocksciState);
        AddressWriter(config).rollback(blocksciState);
        AddressDB(config, config.addressDBFilePath().native()).rollback(blocksciState);
        HashIndexCreator(config, config.hashIndexFilePath().native()).rollback(blocksciState);
    }
}

std::vector<char> HexToBytes(const std::string& hex) {
    std::vector<char> bytes;
    
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(strtol(byteString.c_str(), NULL, 16));
        bytes.push_back(byte);
    }
    
    return bytes;
}

template <typename BlockType>
struct ChainUpdateInfo {
    std::vector<BlockType> blocksToAdd;
    uint32_t splitPoint;
};

uint32_t getStartingTxCount(const blocksci::DataConfiguration &config) {
    blocksci::ChainAccess chain(config, false, blocksci::BlockHeight{0});
    if (chain.blockCount() > 0) {
        auto lastBlock = chain.getBlock(chain.blockCount() - 1);
        return lastBlock->firstTxIndex + lastBlock->numTxes;
    } else {
        return 0;
    }
}

template <typename ParserTag>
void updateChain(const ParserConfiguration<ParserTag> &config, blocksci::BlockHeight maxBlockNum) {
    using namespace std::chrono_literals;
    
    auto chainBlocks = [&]() {
        ChainIndex<ParserTag> index;
        boost::filesystem::ifstream inFile(config.blockListPath(), std::ios::binary);
        if (inFile.good()) {
            boost::archive::binary_iarchive ia(inFile);
            ia >> index;
        }
        index.update(config);
        auto blocks = index.generateChain(maxBlockNum);
        boost::filesystem::ofstream of(config.blockListPath(), std::ios::binary);
        boost::archive::binary_oarchive oa(of);
        oa << index;
        return blocks;
    }();

    blocksci::BlockHeight splitPoint = [&]() {
        blocksci::ChainAccess oldChain(config, false, blocksci::BlockHeight{0});
        blocksci::BlockHeight maxSize = std::min(oldChain.blockCount(), static_cast<blocksci::BlockHeight>(chainBlocks.size()));
        auto splitPoint = maxSize;
        for (blocksci::BlockHeight i{0}; i < maxSize; i++) {
            blocksci::uint256 oldHash = oldChain.getBlock(maxSize - 1 - i)->hash;
            blocksci::uint256 newHash = chainBlocks[static_cast<size_t>(static_cast<int>(maxSize - 1 - i))].hash;
            if (!(oldHash == newHash)) {
                splitPoint = maxSize - 1 - i;
                break;
            }
        }
        
        std::cout << "Starting with chain of " << oldChain.blockCount() << " blocks" << std::endl;
        std::cout << "Removing " << static_cast<blocksci::BlockHeight>(oldChain.blockCount()) - splitPoint << " blocks" << std::endl;
        std::cout << "Adding " << static_cast<blocksci::BlockHeight>(chainBlocks.size()) - splitPoint << " blocks" << std::endl;
        
        return splitPoint;
    }();

    std::vector<BlockInfo<ParserTag>> blocksToAdd{chainBlocks.begin() + static_cast<int>(splitPoint), chainBlocks.end()};
    
    std::ios::sync_with_stdio(false);
    
    rollbackTransactions(splitPoint, config);
    
    if (blocksToAdd.size() == 0) {
        return;
    }
    
    uint32_t startingTxCount = getStartingTxCount(config);
    auto maxBlockHeight = blocksToAdd.back().height;
    
    uint32_t totalTxCount = 0;
    uint32_t totalInputCount = 0;
    uint32_t totalOutputCount = 0;
    for (auto &block : blocksToAdd) {
        totalTxCount += block.nTx;
        totalInputCount += block.inputCount;
        totalOutputCount += block.outputCount;
    }
    
    {
        BlockProcessor processor{startingTxCount, totalTxCount, maxBlockHeight};
        UTXOState utxoState;
        UTXOAddressState utxoAddressState;
        AddressState addressState{config.addressPath()};
        UTXOScriptState utxoScriptState;
        
        utxoAddressState.unserialize(config.utxoAddressStatePath());
        utxoState.unserialize(config.utxoCacheFile().native());
        utxoScriptState.unserialize(config.utxoScriptStatePath().native());
        
        auto it = blocksToAdd.begin();
        auto end = blocksToAdd.end();
        while (it != end) {
            auto prev = it;
            uint32_t newTxCount = 0;
            while (newTxCount < 10000000 && it != end) {
                newTxCount += it->nTx;
                ++it;
            }
            
            decltype(blocksToAdd) nextBlocks{prev, it};
            
            processor.addNewBlocks(config, nextBlocks, utxoState, utxoAddressState, addressState, utxoScriptState);
            
            backUpdateTxes(config);
        }
        
        utxoAddressState.serialize(config.utxoAddressStatePath());
        utxoState.serialize(config.utxoCacheFile().native());
        utxoScriptState.serialize(config.utxoScriptStatePath().native());
    }
}

void updateHashDB(const ParserConfigurationBase &config) {
    blocksci::ChainAccess chain{config, false, blocksci::BlockHeight{0}};
    blocksci::ScriptAccess scripts{config};
    
    blocksci::State updateState{chain, scripts};
    HashIndexCreator db(config, config.hashIndexFilePath().native());
    
    std::cout << "Updating hash index\n";
    
    db.prepareUpdate();
    db.runUpdate(updateState);
    db.tearDown();
}

void updateAddressDB(const ParserConfigurationBase &config) {
    blocksci::ChainAccess chain{config, false, blocksci::BlockHeight{0}};
    blocksci::ScriptAccess scripts{config};
    
    blocksci::State updateState{chain, scripts};
    AddressDB db(config, config.addressDBFilePath().native());
    
    std::cout << "Updating address index\n";
    
    db.prepareUpdate();
    db.runUpdate(updateState);
    db.tearDown();
}

int main(int argc, const char * argv[]) {
    
    namespace po = boost::program_options;
    std::string dataDirectoryString;
    int maxBlockNum;
    po::options_description general("Options");
    general.add_options()
    ("help", "Print help messages")
    ("output-directory", po::value<std::string>(&dataDirectoryString)->required(), "Path to output parsed data")
    ("max-block", po::value<int>(&maxBlockNum)->default_value(0), "Max block to scan up to")
    ("update-indexes", "Update indexes to latest chain state")
    ("update-address-index", "Update address index to latest state")
    ;

    std::string username;
    std::string password;
    std::string address;
    int port;
    po::options_description rpcOptions("RPC options");
    rpcOptions.add_options()
    ("username", po::value<std::string>(&username), "RPC username")
    ("password", po::value<std::string>(&password), "RPC password")
    ("address", po::value<std::string>(&address)->default_value("127.0.0.1"), "RPC address")
    ("port", po::value<int>(&port)->default_value(9998), "RPC port")
    ;
    
    #ifdef BLOCKSCI_FILE_PARSER
    std::string bitcoinDirectoryString;
    po::options_description fileOptions("File parser options");
    fileOptions.add_options()
    ("coin-directory", po::value<std::string>(&bitcoinDirectoryString), "Path to cryptocurrency directory")
    ;
    #endif
    
    po::options_description all("Allowed options");
    all.add(general);
    #ifdef BLOCKSCI_FILE_PARSER
    all.add(fileOptions);
    #endif
    #ifdef BLOCKSCI_RPC_PARSER
    all.add(rpcOptions);
    #endif
    
    bool validDisk = false;
    bool validRPC = false;
    boost::filesystem::path bitcoinDirectory;
    boost::filesystem::path dataDirectory;
    
    try {
        po::variables_map vm;
        
        po::store(po::parse_command_line(argc, argv, all), vm); // can throw
        
        /** --help option
         */
        if ( vm.count("help")  )
        {
            std::cout << "Basic Command Line Parameter App" << std::endl
            << all << std::endl;
            return 1;
        }
        
        po::notify(vm); // throws on error, so do after help in case
        // there are any problems
        
        dataDirectory = {dataDirectoryString};
        dataDirectory = boost::filesystem::absolute(dataDirectory);
        auto configFile = dataDirectory/"config.ini";
        if (boost::filesystem::exists(configFile)) {
            boost::filesystem::ifstream configStream{configFile};
            po::store(po::parse_config_file(configStream, all, true), vm);
        }

        bool diskEnabled = false;
        bool rpcEnabled = false;
        
        #ifdef BLOCKSCI_FILE_PARSER
        diskEnabled = true;
        #endif
        
        #ifdef BLOCKSCI_RPC_PARSER
        rpcEnabled = true;
        #endif
        
        if (vm.count("update-indexes")) {
            ParserConfigurationBase config{dataDirectory};
            updateAddressDB(config);
            updateHashDB(config);
            return 0;
        }
        
        if (vm.count("update-hash-index")) {
            ParserConfigurationBase config{dataDirectory};
            updateHashDB(config);
            return 0;
        }
        
        if (vm.count("update-address-index")) {
            ParserConfigurationBase config{dataDirectory};
            updateAddressDB(config);
            return 0;
        }
        
        if (diskEnabled) {
            if (vm.count("coin-directory")) {
                validDisk = true;
            }
        }
        
        bool enabledSomeRPC = false;
        bool enabledAllRPC = false;
        if (rpcEnabled) {
            enabledSomeRPC = vm.count("username") || vm.count("password");
            enabledAllRPC = vm.count("username") && vm.count("password");
            if (enabledSomeRPC && !enabledAllRPC) {
                throw std::logic_error(std::string("Must specify all RPC username and password"));
            } else if (enabledAllRPC) {
                validRPC = true;
            }
        }
        
        if (rpcEnabled && diskEnabled) {
            if (!validDisk && !validRPC) {
                throw std::logic_error(std::string("Must specify disk or RPC options"));
            }
        } else if (diskEnabled) {
            if (!validDisk) {
                throw std::logic_error(std::string("Must specify disk options"));
            }
        } else if (rpcEnabled) {
            if (!validRPC) {
                throw std::logic_error(std::string("Must specify RPC options"));
            }
        }
        
        boost::property_tree::ptree rootPTree;
        
        rootPTree.put("version", blocksci::dataVersion);
        
        if (validDisk) {
            bitcoinDirectory = {bitcoinDirectoryString};
            bitcoinDirectory = boost::filesystem::absolute(bitcoinDirectory);
            rootPTree.put("coin-directory", bitcoinDirectory.native());
        }
        
        if (validRPC) {
            rootPTree.put("username", username);
            rootPTree.put("password", password);
            rootPTree.put("address", address);
            rootPTree.put("port", port);
        }
        
        if(!(boost::filesystem::exists(dataDirectory))){
            boost::filesystem::create_directory(dataDirectory);
        }
        
        boost::filesystem::ofstream configStream{configFile};
        boost::property_tree::write_ini(configStream, rootPTree);
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << all << std::endl;
        return -1;
    }
    
    
    if (validDisk) {
        #ifdef BLOCKSCI_FILE_PARSER
        ParserConfiguration<FileTag> config{bitcoinDirectory, dataDirectory};
        updateChain(config, blocksci::BlockHeight{maxBlockNum});
        updateHashDB(config);
        updateAddressDB(config);
        #endif
    } else if (validRPC) {
        #ifdef BLOCKSCI_RPC_PARSER
        ParserConfiguration<RPCTag> config(username, password, address, port, dataDirectory);
        updateChain(config, blocksci::BlockHeight{maxBlockNum});
        #endif
    }
    
    
    return 0;
}
