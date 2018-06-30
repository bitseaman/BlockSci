//
//  script_info.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef script_info_hpp
#define script_info_hpp

#include "address_types.hpp"

#include <boost/variant/variant_fwd.hpp>

#include <tuple>
#include <sstream>
#include <stdio.h>

namespace blocksci {
    
    struct PubkeyData;
    struct PubkeyHashData;
    struct ScriptHashData;
    struct MultisigData;
    struct NonstandardScriptData;
    struct RawData;
    
    template <typename T>
    struct FixedSize;
    
    template <typename ...T>
    struct Indexed;
    
    template <AddressType::Enum>
    struct AddressInfo;
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEY> {
        static constexpr char typeName[] = "pubkey";
        static constexpr int addressType = 0;
        static constexpr bool spendable = true;
        static constexpr bool hasNestedAddresses = false;
        static constexpr bool deduped = true;
        static constexpr AddressType::Enum addressAddressType = AddressType::Enum::PUBKEYHASH;
        using storage = FixedSize<PubkeyData>;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::PUBKEYHASH> {
        static constexpr char typeName[] = "pubkeyhash";
        static constexpr int addressType = 0;
        static constexpr bool spendable = true;
        static constexpr bool hasNestedAddresses = false;
        static constexpr bool deduped = true;
        static constexpr AddressType::Enum addressAddressType = AddressType::Enum::PUBKEYHASH;
        using storage = FixedSize<PubkeyHashData>;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::SCRIPTHASH> {
        static constexpr char typeName[] = "scripthash";
        static constexpr int addressType = 1;
        static constexpr bool spendable = true;
        static constexpr bool hasNestedAddresses = true;
        static constexpr bool deduped = true;
        static constexpr AddressType::Enum addressAddressType = AddressType::Enum::SCRIPTHASH;
        using storage = FixedSize<ScriptHashData>;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::MULTISIG> {
        static constexpr char typeName[] = "multisig";
        static constexpr int addressType = 2;
        static constexpr bool spendable = true;
        static constexpr bool hasNestedAddresses = true;
        static constexpr bool deduped = true;
        static constexpr AddressType::Enum addressAddressType = AddressType::Enum::MULTISIG;
        using storage = Indexed<MultisigData>;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NONSTANDARD> {
        static constexpr char typeName[] = "nonstandard";
        static constexpr int addressType = -1;
        static constexpr bool spendable = true;
        static constexpr bool hasNestedAddresses = false;
        static constexpr bool deduped = false;
        static constexpr AddressType::Enum addressAddressType = AddressType::Enum::NONSTANDARD;
        using storage = Indexed<NonstandardScriptData,NonstandardScriptData>;
    };
    
    template <>
    struct AddressInfo<AddressType::Enum::NULL_DATA> {
        static constexpr char typeName[] = "nulldata";
        static constexpr int addressType = -1;
        static constexpr bool spendable = false;
        static constexpr bool hasNestedAddresses = false;
        static constexpr bool deduped = false;
        static constexpr AddressType::Enum addressAddressType = AddressType::Enum::NULL_DATA;
        using storage = Indexed<RawData>;
    };
    
    std::string GetTxnOutputType(AddressType::Enum t);
    
    template<AddressType::Enum AddressType>
    struct ScriptTag {
        static constexpr AddressType::Enum type = AddressType;
    };
    
    namespace internal {
        template<typename T>
        struct to_variant;
        
        template<typename... Ts>
        struct to_variant<std::tuple<Ts...>>
        {
            using type = boost::variant<Ts...>;
        };
        
        template<template<AddressType::Enum> class K, typename T>
        struct to_script_type;
        
        template<template<AddressType::Enum> class K, AddressType::Enum Type>
        struct to_script_type<K, ScriptTag<Type>> {
            using type = K<Type>;
        };
        
        template <template<AddressType::Enum> class K, typename... Types>
        struct to_script_type<K, std::tuple<Types...>> {
            using type = std::tuple<typename to_script_type<K, Types>::type...>;
        };
        
        template<int I, class Tuple, typename F> struct for_each_impl {
            static void for_each(Tuple& t, F f) {
                for_each_impl<I - 1, Tuple, F>::for_each(t, f);
                f(std::get<I>(t));
            }
        };
        template<class Tuple, typename F> struct for_each_impl<0, Tuple, F> {
            static void for_each(Tuple& t, F f) {
                f(std::get<0>(t));
            }
        };
        
        template <class F, size_t... Is>
        constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
            return f(std::integral_constant<size_t, Is> {}...);
        }
        
        template <size_t N, class F>
        constexpr auto index_apply(F f) {
            return index_apply_impl(f, std::make_index_sequence<N>{});
        }
        
        template <template<AddressType::Enum> class Functor, std::size_t... Is>
        constexpr auto make_dynamic_table_impl(std::index_sequence<Is...>) -> std::array<decltype(&Functor<AddressType::Enum::PUBKEY>::f), sizeof...(Is)> {
            return {{Functor<AddressType::all[Is]>::f...}};
        }
        
        template <template<AddressType::Enum> class Functor, std::size_t... Is>
        constexpr auto make_static_table_impl(std::index_sequence<Is...>) -> std::array<decltype(Functor<AddressType::Enum::PUBKEY>::f()), sizeof...(Is)> {
            return {{Functor<AddressType::all[Is]>::f()...}};
        }
    }
    
    template <template<AddressType::Enum> class Functor>
    constexpr auto make_static_table() {
        return internal::make_static_table_impl<Functor>(std::make_index_sequence<AddressType::all.size()>{});
    }
    
    template <template<AddressType::Enum> class Functor>
    constexpr auto make_dynamic_table() {
        return internal::make_dynamic_table_impl<Functor>(std::make_index_sequence<AddressType::all.size()>{});
    }
    
    template <class Tuple, class F>
    constexpr auto apply(Tuple t, F f) {
        return internal::index_apply<std::tuple_size<Tuple>{}>([&](auto... Is) {
            return std::make_tuple(f(std::get<Is>(t))...);
        });
    }
    
    
    template<class Tuple, typename F>
    void for_each(Tuple& t, F f) {
        internal::for_each_impl<std::tuple_size<Tuple>::value - 1, Tuple, F>::for_each(t, f);
    }
    
    template <template<AddressType::Enum> class K, typename... Types>
    using to_script_variant_t = typename internal::to_variant<typename internal::to_script_type<K, Types...>::type>::type;

    
    template<blocksci::AddressType::Enum type>
    struct SpendableFunctor {
        static constexpr bool f() {
            return AddressInfo<type>::spendable;
        }
    };
    
    template<blocksci::AddressType::Enum type>
    struct AddressAddressTypeFunctor {
        static constexpr AddressType::Enum f() {
            return AddressInfo<type>::addressAddressType;
        }
    };
    
    template<AddressType::Enum type>
    struct HasNestedAddressesFunctor {
        static constexpr bool f() {
            return AddressInfo<type>::hasNestedAddresses;
        }
    };
    
    template<AddressType::Enum type>
    struct DBTypeFunctor {
        static constexpr int f() {
            return AddressInfo<type>::addressType;
        }
    };
    
    inline auto getAddressTypes() {
        return internal::index_apply<AddressType::all.size()>([](auto... Is) {
            return std::make_tuple(ScriptTag<std::get<Is>(AddressType::all)>{}...);
        });
    }
    
    using AddressInfoList = decltype(getAddressTypes());
    
    static constexpr auto spendableTable = blocksci::make_static_table<SpendableFunctor>();
    static constexpr auto addressAddressTypeTable = blocksci::make_static_table<AddressAddressTypeFunctor>();
    static constexpr auto nestedAddressTable = blocksci::make_static_table<HasNestedAddressesFunctor>();
    static constexpr auto dbTypeTable = blocksci::make_static_table<DBTypeFunctor>();
    
    constexpr bool isSpendable(AddressType::Enum t) {
        
        auto index = static_cast<size_t>(t);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return spendableTable[index];
    }
    
    constexpr AddressType::Enum addressAddressType(AddressType::Enum t) {
        
        auto index = static_cast<size_t>(t);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return addressAddressTypeTable[index];
    }
    
    constexpr bool hasNestedAddresses(AddressType::Enum t) {
        
        auto index = static_cast<size_t>(t);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return nestedAddressTable[index];
    }
    
    constexpr int getDBType(AddressType::Enum t) {
        
        auto index = static_cast<size_t>(t);
        if (index >= AddressType::size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return dbTypeTable[index];
    }
}

#endif /* script_info_hpp */
