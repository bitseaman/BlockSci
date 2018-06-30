//
//  chain_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef chain_access_hpp
#define chain_access_hpp

#include <blocksci/file_mapper.hpp>
#include <blocksci/bitcoin_uint256.hpp>

#include <boost/range/iterator_range.hpp>
#include <stdio.h>

namespace blocksci {
    class ReorgException : public std::runtime_error {
    public:
        ReorgException() : std::runtime_error("") {}
    };
    
    struct Block;
    struct Output;
    struct Input;
    struct DataConfiguration;
    struct RawTransaction;
    
    
    class ChainAccess {
        FixedSizeFileMapper<Block> blockFile;
        SimpleFileMapper<> blockCoinbaseFile;
        
        IndexedFileMapper<boost::iostreams::mapped_file::mapmode::readonly, RawTransaction> txFile;
        FixedSizeFileMapper<uint256> txHashesFile;
        IndexedFileMapper<boost::iostreams::mapped_file::mapmode::readonly, uint64_t> txVpubFile;

        
        uint32_t _maxLoadedTx;
        
        const uint256 *lastBlockHashDisk;
        uint256 lastBlockHash;
        uint32_t maxHeight;
        
        void reorgCheck() const;
        
        bool errorOnReorg;
        
        const FixedSizeFileMapper<Block> &getBlockFile() const {
            return blockFile;
        }
        
    public:
        ChainAccess(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored);
        
        uint32_t maxLoadedTx() const;
        
        uint32_t getBlockHeight(uint32_t txIndex) const;
        const Block &getBlock(uint32_t blockHeight) const;
        const boost::iterator_range<const Block *> getBlocks() const;
        
        const char *getTxPos(uint32_t index) const;
        
        const RawTransaction *getTx(uint32_t index) const;
        
        const Output &getOutput(uint32_t txIndex, uint16_t outputNum) const;
        const Input &getInput(uint32_t txIndex, uint16_t inputNum) const;
        
        std::vector<uint64_t> getVpubold(uint32_t txIndex, uint16_t vpubNum) const;
        std::vector<uint64_t> getVpubnew(uint32_t txIndex, uint16_t vpubNum) const;
        
        size_t txCount() const;
        
        std::vector<unsigned char> getCoinbase(uint64_t offset) const;
        
        const FixedSizeFileMapper<uint256> &getTxHashesFile() const {
            reorgCheck();
            return txHashesFile;
        }
    };
}


#endif /* chain_access_hpp */
