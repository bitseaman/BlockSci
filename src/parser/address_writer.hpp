//
//  address_writer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef address_writer_hpp
#define address_writer_hpp

#include "parser_configuration.hpp"
#include "script_output.hpp"
#include "script_input.hpp"

#include <blocksci/file_mapper.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_data.hpp>

#include <boost/variant/variant.hpp>

#include <stdio.h>

template<typename T>
struct ScriptFileType;

template<typename T>
struct ScriptFileType<blocksci::FixedSize<T>> {
    using type = blocksci::FixedSizeFileMapper<T, boost::iostreams::mapped_file::readwrite>;
};

template<typename ...T>
struct ScriptFileType<blocksci::Indexed<T...>> {
    using type = blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, T...>;
};


template<blocksci::AddressType::Enum type>
using ScriptFileType_t = typename ScriptFileType<typename blocksci::AddressInfo<type>::storage>::type;

template<blocksci::AddressType::Enum type>
struct ScriptFile : public ScriptFileType_t<type> {
    using ScriptFileType_t<type>::ScriptFileType_t;
};

template<blocksci::AddressType::Enum type>
struct ScriptOutput;

template<blocksci::AddressType::Enum type>
struct ScriptInput;

class AddressWriter {
    using ScriptFilesTuple = blocksci::internal::to_script_type<ScriptFile, blocksci::AddressInfoList>::type;
    
    ScriptFilesTuple scriptFiles;
    
    template<blocksci::AddressType::Enum type>
    void serializeImp(const ScriptInput<type> &input);
    
    template<blocksci::AddressType::Enum type>
    void serializeImp(const ScriptOutput<type> &output);
    
public:
    
    template <blocksci::AddressType::Enum type>
    ScriptFile<type> &getFile() {
        return std::get<ScriptFile<type>>(scriptFiles);
    }
    
    template <blocksci::AddressType::Enum type>
    const ScriptFile<type> &getFile() const {
        return std::get<ScriptFile<type>>(scriptFiles);
    }
    
    template<blocksci::AddressType::Enum type>
    void serialize(const ScriptOutput<type> &output) {
        serializeImp(output);
    }
    
    template<blocksci::AddressType::Enum type>
    void serialize(const ScriptInput<type> &input) {
        serializeImp(input);
    }
    
    template<blocksci::AddressType::Enum type>
    void truncate(uint32_t index) {
        getFile<type>().truncate(index);
    }
    
    void truncate(blocksci::AddressType::Enum type, uint32_t index);
    
    void serialize(const ScriptOutputType &output);
    void serialize(const ScriptInputType &input);
    
    AddressWriter(const ParserConfiguration &config);
};

#endif /* address_writer_hpp */
