//
//  parser_configuration.h
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef parser_configuration_h
#define parser_configuration_h

#include "config.hpp"
#include "parser_fwd.hpp"

#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/util/data_configuration.hpp>

#include <boost/filesystem/path.hpp>

#include <functional>

struct ParserConfigurationBase : public blocksci::DataConfiguration {
    ParserConfigurationBase();
    ParserConfigurationBase(const boost::filesystem::path &dataDirectory_);
    
    boost::filesystem::path parserDirectory() const {
        return dataDirectory/"parser";
    }
    
    boost::filesystem::path utxoCacheFile() const {
        return parserDirectory()/"utxoCache.dat";
    }
    
    boost::filesystem::path utxoAddressStatePath() const {
        return parserDirectory()/"utxoAddressState";
    }
    
    boost::filesystem::path utxoScriptStatePath() const {
        return parserDirectory()/"utxoScriptState";
    }
    
    
    
    boost::filesystem::path addressPath() const {
        return parserDirectory()/"address";
    }
    
    boost::filesystem::path blockListPath() const {
        return parserDirectory()/"blockList.dat";
    }
    
    boost::filesystem::path txUpdatesFilePath() const {
        return parserDirectory()/"txUpdates";
    }
    
    bool witnessActivatedAtHeight(uint32_t blockHeight) const;
};

#ifdef BLOCKSCI_FILE_PARSER
template<>
struct ParserConfiguration<FileTag> : public ParserConfigurationBase {
    ParserConfiguration();
    ParserConfiguration(const boost::filesystem::path &bitcoinDirectory_, const boost::filesystem::path &dataDirectory_);
    
    boost::filesystem::path bitcoinDirectory;
    uint32_t blockMagic;
    std::function<blocksci::uint256(const char *data, unsigned long len)> workHashFunction;
    
    
    boost::filesystem::path pathForBlockFile(int fileNum) const;
};
#endif

#ifdef BLOCKSCI_RPC_PARSER

class BitcoinAPI;

template<>
struct ParserConfiguration<RPCTag> : public ParserConfigurationBase {
    ParserConfiguration();
    ParserConfiguration(std::string username, std::string password, std::string address, int port, const boost::filesystem::path &dataDirectory_);
    
    std::string username;
    std::string password;
    std::string address;
    int port;
    
    BitcoinAPI createBitcoinAPI() const;
};
#endif


#endif /* parser_configuration_h */
