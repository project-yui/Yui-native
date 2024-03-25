#include "../include/convert.hh"
#include "napi.h"
#include "spdlog/spdlog.h"

namespace nt_convert {
    static std::map<int32_t, convert> convertTable;

    /**
     * @brief 文字元素转换
     * 
     * @param dest 
     * @param src 
     */
    void textElement2Protobuf(nt_msg::Element *dest, Napi::Object &src)
    {
        dest->set_elementtype(nt_msg::Element_MsgType_MSG_TYPE_TEXT);
        
        auto elementId = src.Get("elementId").As<Napi::String>();
        dest->set_elementid(atol(elementId.Utf8Value().c_str()));

        auto textElement = src.Get("textElement").As<Napi::Object>();
        auto textStr = textElement.Get("content").As<Napi::String>();
        dest->set_textstr(textStr.Utf8Value());
        dest->set_attype(0);
    }

    ElementConverter::ElementConverter() {
        this->convertTable.emplace(std::pair<int32_t, convert>(1, textElement2Protobuf));
    }
    ElementConverter::~ElementConverter() {

    }
    ElementConverter& ElementConverter::getInstance() {
        return ElementConverter::instance;
    }
    void ElementConverter::toProtobuf(Napi::Array &data, std::vector<char>& output)
    {
        spdlog::debug("convert elements to protobuf...");
        int cnt = data.Length();
        nt_msg::Elements elems;
        for (int i=0; i < cnt; i++) {
            auto element = data.Get(i).As<Napi::Object>();
            
            auto elem = elems.add_elem();
            auto elementType = element.Get("elementType").As<Napi::Number>().Int32Value();
            if (convertTable.find(elementType) != convertTable.end())
            {
                // void convert(nt_msg::Element *, Napi::Object &);
                // convert(elem, element);
                convertTable[elementType](elem, element);
            }
            else {
                spdlog::warn("Element type is {}, it is not supported.", elementType);
            }

        }
        std::string out;
        elems.SerializeToString(&out);
        for (int j=0; j < out.length(); j++) {
            output.push_back(out[j]);
        }
    }
}