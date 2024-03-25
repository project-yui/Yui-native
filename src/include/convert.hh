#ifndef __CONVERT_HH__
#define __CONVERT_HH__
#include <napi.h>
#include <map>
#include "../proto/message.pb.h"

namespace nt_convert {
    typedef void (* convert)(nt_msg::Element *, Napi::Object &);

    class ElementConverter {
        public:
        ElementConverter();
        ~ElementConverter();

        /**
         * @brief Get the Instance object
         * 
         * @return ElementConverter& 
         */
        static ElementConverter& getInstance();

        /**
         * @brief 转换消息元素为protobuf格式
         * 
         * @param data 
         * @param output 
         */
        void toProtobuf(Napi::Array &data, std::vector<char>& output);

        private:
        ElementConverter(const ElementConverter&) = delete;
        void operator=(const ElementConverter&) = delete;
        std::map<int32_t, convert> convertTable;

        static ElementConverter instance;
    };
    
}

#endif