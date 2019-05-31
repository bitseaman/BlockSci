//
//  script_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef script_info_hpp
#define script_info_hpp

#include "core_fwd.hpp"
#include "dedup_address_type.hpp"

namespace blocksci {
    
    template <typename T>
    struct BLOCKSCI_EXPORT FixedSize;
    
    template <typename ...T>
    struct BLOCKSCI_EXPORT Indexed;
    
    template <>
    struct BLOCKSCI_EXPORT ScriptInfo<DedupAddressType::PUBKEY> {
        using outputType = PubkeyData;
        using storage = FixedSize<PubkeyData>;
    };
    
    template <>
    struct BLOCKSCI_EXPORT BLOCKSCI_EXPORT ScriptInfo<DedupAddressType::SCRIPTHASH> {
        using outputType = ScriptHashData;
        using storage = FixedSize<ScriptHashData>;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptInfo<DedupAddressType::MULTISIG> {
        using outputType = MultisigData;
        using storage = Indexed<MultisigData>;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptInfo<DedupAddressType::NONSTANDARD> {
        using outputType = NonstandardScriptData;
        using storage = Indexed<NonstandardScriptData,NonstandardSpendScriptData>;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptInfo<DedupAddressType::NULL_DATA> {
        using outputType = RawData;
        using storage = Indexed<RawData>;
    };
} // namespace blocksci

#endif /* script_info_hpp */
