#include "../include/convert.hh"
#include "napi.h"
#include "spdlog/spdlog.h"

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
        dest->set_elementtype(nt_msg::Element_MsgType_MSG_TYPE_TEXT);
        
        // elementId
        auto elementId = src.Get("elementId").As<Napi::String>();
        dest->set_elementid(atol(elementId.Utf8Value().c_str()));

        auto textElement = src.Get("textElement").As<Napi::Object>();
        auto textStr = textElement.Get("content").As<Napi::String>();
        dest->set_textstr(textStr.Utf8Value());
        dest->set_attype(0);
    }

    /**
     * @brief 图片元素转换
     * 
     * @param dest protobuf对象
     * @param src NAPI对象
     */
    void picElement2Protobuf(nt_msg::Element *dest, Napi::Object &src)
    {
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
        dest->set_elementtype(nt_msg::Element_MsgType_MSG_TYPE_TEXT);
        
        // elementId
        auto elementId = src.Get("elementId").As<Napi::String>();
        dest->set_elementid(atol(elementId.Utf8Value().c_str()));

        auto picElement = src.Get("picElement").As<Napi::Object>();

        {
            auto fileName = picElement.Get("fileName").As<Napi::String>();
            dest->set_filename(fileName.Utf8Value());

            auto fileSize = picElement.Get("fileSize").As<Napi::Number>();
            dest->set_filesize(fileSize.Int32Value());

            auto picWidth = picElement.Get("picWidth").As<Napi::Number>();
            dest->set_picwidth(picWidth.Int32Value());
            
            auto picHeight = picElement.Get("picHeight").As<Napi::Number>();
            dest->set_picheight(picHeight.Int32Value());
            
            auto picType = picElement.Get("picType").As<Napi::Number>();
            dest->set_pictype(picType.Int32Value());
            
            auto fileUuid = picElement.Get("fileUuid").As<Napi::String>();
            dest->set_fileuuid(fileUuid.Utf8Value());
            
            auto originImageUrl = picElement.Get("originImageUrl").As<Napi::String>();
            dest->set_originimageurl(originImageUrl.Utf8Value());
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