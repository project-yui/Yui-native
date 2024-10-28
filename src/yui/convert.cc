#include "../include/convert.hh"
#include "napi.h"
#include "spdlog/spdlog.h"
#include <string>

namespace nt_convert {
    
    /**
     * 消息元素类型
     * 
     * 1 - 纯文本&@ textElement
     * 
     * 2 - 图片 picElement
     * 
     * 5 - 视频 videoElement
     * 
     * 6 - 表情 faceElement
     * 
     * 7 - 引用回复 replyElement
     * 
     * 8 - 提示消息 [撤回]
     * 
     * 10 - ArkElement
     * 
     * 11 - 商城表情 marketFaceElement
     * 
     * 16 - 转发消息 multiForwardMsgElement
     * 
     */
    static std::map<int32_t, convert> convertTable;

    /**
     * @brief 文字元素转换
     * 
     * @param dest 
     * @param src 
     */
    void textElement2Protobuf(nt_msg::Element *dest, Napi::Object &src)
    {
        spdlog::info("convert textElement");
        dest->set_elementtype(nt_msg::Element_MsgType_MSG_TYPE_TEXT);
        
        // elementId
        auto elementId = src.Get("elementId").As<Napi::String>();
        dest->set_elementid(atol(elementId.Utf8Value().c_str()));

        auto textElement = src.Get("textElement").As<Napi::Object>();
        auto textStr = textElement.Get("content").As<Napi::String>();
        dest->set_textstr(textStr.Utf8Value());
        dest->set_attype(0);
    }
    void hex_string_to_byte_array(const char *hex_string, char *byte_array, int byte_array_size) {
        for (int i = 0; i < byte_array_size; i++) {
            #ifdef _WIN32
            sscanf_s(hex_string + 2 * i, "%2hhx", &byte_array[i]);
            #elif defined (__linux__)
            sscanf(hex_string + 2 * i, "%2hhx", &byte_array[i]);
            #endif
        }
        byte_array[byte_array_size / 2] = '\0';
    }
    /**
     * @brief 图片元素转换
     * 
     * @param dest protobuf对象
     * @param src NAPI对象
     */
    void picElement2Protobuf(nt_msg::Element *dest, Napi::Object &src)
    {
        spdlog::info("convert picElement");
        // export interface PicElementType {
        //     /**
        //     * 0 - 普通图片
        //     * 
        //     * 1 - 表情图片
        //     */
        //     picSubType: 0 | 1
        //     fileName: `{${string}}.${string}`
        //     fileSize: `${number}`
        //     picWidth: number
        //     picHeight: number
        //     original: boolean
        //     md5HexStr: string
        //     sourcePath: string
        //     thumbPath: {}
        //     transferStatus: number
        //     progress: number
        //     picType: number
        //     invalidState: number
        //     fileUuid: `${number}`
        //     fileSubId: string
        //     thumbFileSize: number
        //     fileBizId:null
        //     downloadIndex:null
        //     summary: string
        //     emojiFrom: null
        //     emojiWebUrl: null
        //     emojiAd:{
        //     url: string
        //     desc: string
        //     }
        //     emojiMall:{
        //     packageId: number
        //     emojiId: number
        //     },
        //     emojiZplan:{
        //     actionId: 0,
        //     actionName: string,
        //     actionType: 0,
        //     playerNumber: 0,
        //     peerUid: `${number}`
        //     bytesReserveInfo: string
        //     },
        //     originImageMd5: string

        //     /**
        //     * 图片网络地址
        //     * 
        //     * 没有host, https://gchat.qpic.cn/
        //     * 
        //     * /gchatpic_new/发送者QQ/群号-uuid-MD5/0
        //     */
        //     originImageUrl: string
        //     import_rich_media_context: null,
        //     isFlashPic: boolean
        // }
        // {
        //     "elementType": 2,
        //     "elementId": "7756194487841896",
        //     "extBufForUI": "0x",
        //     "picElement": {
        //         "md5HexStr": "2330af0b508245a2709620ffd4f2709a",
        //         "picWidth": 960,
        //         "picHeight": 782,
        //         "fileName": "{2330af0b508245a2709620ffd4f2709a}.jpg",
        //         "fileSize": "74566",
        //         "original": true,
        //         "picSubType": 0,
        //         "sourcePath": "D:\\data\\tim/nt_qq\\nt_data\\Pic\\2024-03\\Ori\\2330af0b508245a2709620ffd4f2709a.jpg",
        //         "thumbPath": {},
        //         "picType": 1001,
        //         "fileUuid": "78770549763",
        //         "fileSubId": "",
        //         "thumbFileSize": 0,
        //         "summary": "",
        //         "emojiFrom": null,
        //         "emojiWebUrl": null,
        //         "emojiAd": {
        //             "url": "",
        //             "desc": ""
        //         },
        //         "emojiMall": {
        //             "packageId": 0,
        //             "emojiId": 0
        //         },
        //         "emojiZplan": {
        //             "actionId": 0,
        //             "actionName": "",
        //             "actionType": 0,
        //             "playerNumber": 0,
        //             "peerUid": "1",
        //             "bytesReserveInfo": ""
        //         },
        //         "originImageMd5": "",
        //         "originImageUrl": "",
        //         "import_rich_media_context": null,
        //         "isFlashPic": false,
        //         "transferStatus": 1,
        //         "progress": 100,
        //         "invalidState": 0,
        //         "fileBizId": null,
        //         "downloadIndex": null
        //     }
        
        // elementId
        auto elementId = src.Get("elementId").As<Napi::String>();
        dest->set_elementid(atoll(elementId.Utf8Value().c_str()));
        dest->set_elementtype(nt_msg::Element_MsgType_MSG_TYPE_PIC);

        auto picElement = src.Get("picElement").As<Napi::Object>();

        {
            auto fileName = picElement.Get("fileName").As<Napi::String>();
            dest->set_filename(fileName.Utf8Value());

            auto fileSize = picElement.Get("fileSize").As<Napi::String>();
            dest->set_filesize(atol(fileSize.Utf8Value().c_str()));

            // MD5关键，服务器依据MD5获取图片
            auto md5 = picElement.Get("md5HexStr").As<Napi::String>().Utf8Value();
            hex_string_to_byte_array(md5.c_str(), &md5[0], md5.length());
            dest->set_md5(md5.c_str());

            auto picWidth = picElement.Get("picWidth").As<Napi::Number>();
            dest->set_picwidth(picWidth.Int32Value());
            
            auto picHeight = picElement.Get("picHeight").As<Napi::Number>();
            dest->set_picheight(picHeight.Int32Value());

            dest->set_unknown_45413(0);
            dest->set_unknown_45414(0);
            
            auto picType = picElement.Get("picType").As<Napi::Number>();
            dest->set_pictype(picType.Int32Value());

            dest->set_unknown_45418(0);
            
            auto fileUuid = picElement.Get("fileUuid").As<Napi::String>();
            dest->set_fileuuid(fileUuid.Utf8Value());
            
            auto originImageUrl = picElement.Get("originImageUrl").As<Napi::String>().Utf8Value();
            std::string from = "/0?";
            dest->set_originimageurl(originImageUrl);
            size_t start_pos = originImageUrl.find(from);
            if (start_pos != std::string::npos)
            {
                std::string image198;
                image198.assign(originImageUrl);
                std::string image720;
                image720.assign(originImageUrl);
                image198.replace(start_pos, from.length(), "/198?");
                dest->set_imageurl198(image198);
                spdlog::info("image198: {}", image198.c_str());
                image720.replace(start_pos, from.length(), "/720?");
                dest->set_imageurl720(image720);
                spdlog::info("image720: {}", image720.c_str());
            }
            dest->set_unknown_45805(0);
            // dest->set_unknown_45806(114514);
            dest->set_unknown_45807(80);
            dest->set_unknown_45829(0);

        }
        
    }
    ElementConverter::ElementConverter() {
        // 文本
        this->convertTable.emplace(std::pair<int32_t, convert>(1, textElement2Protobuf));
        // 图片
        this->convertTable.emplace(std::pair<int32_t, convert>(2, picElement2Protobuf));
    }
    ElementConverter::~ElementConverter() {

    }
    ElementConverter& ElementConverter::getInstance() {
        static ElementConverter instance;
        return instance;
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
                spdlog::info("call to parser...");
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