#include <string.h>
#include <stdio.h>
#include <fstream>
#include <string>

#include "nlohmann/json.hpp"

#include "dbProxy.h"
#include "recognize.h"
#include "faceEngine.h"
#include "common.h"
    
extern "C" 
{
    // 引擎实例
    struct Instance 
    {
        // 基本引擎
        FaceEngine m_faceEngine;

        // 识别器
        Recognize m_recognizer;

        // 数据库管理器
        DBProxy m_dbProxy;

        // 配置表
        nlohmann::json m_setting;
    };

    // /////////////////////////////////////////////////////////
    // 图片的注册结果
    struct RegisterResult
    {
        // 识别到的对方名字
        char name[100];   

        // 图片完整路径
        char imagePath[1024];

        int left;
        int top;
        int right;
        int bottom;
    };
    struct RegisterResultResponse
    {
        int count;
        RegisterResult* elems;
    };

    // /////////////////////////////////////////////////////////
    // 图片的识别结果
    struct RecognizeResult
    {
         // 识别到的对方名字
        char name[100];   

        // 图片完整路径
        char imagePath[1024];

        int left;
        int top;
        int right;
        int bottom;

        // 识别到的对方名字
        char personName[100];   
        
        // 识别置信等级
        float confidenceLevel;          
    };
    struct RecognizeResultResponse
    {
        int count;
        RecognizeResult* elems;
    };

    void* fe_init()
    {
        Instance* instance = new Instance();
        auto& faceEngine = instance->m_faceEngine;
        auto& settings = instance->m_setting;
        auto& dbProxy = instance->m_dbProxy;
        auto& recognizer = instance->m_recognizer;

        faceEngine.Init();

        // 初始化配置
        std::ifstream fin("setting.json");
        fin >> settings; 
        fin.close();

        // 初始化sdk激活
        const std::string appId(settings["app_id"]);
        const std::string sdkKey(settings["sdk_key"]);
        faceEngine.DumpActivationInfos(const_cast<char*>(appId.c_str()), const_cast<char*>(sdkKey.c_str()), NULL);

        // 初始化脸谱数据库
        bool ok = dbProxy.Init(settings["db_path"]);
        if(!ok){
            exit(1);
        }

        // 加载已有脸谱
        recognizer.LoadAllFaces(dbProxy);

        return instance;
    }

    void fe_release(void* pInstance)
    {
        auto instance = static_cast<Instance*>(pInstance);
        if(instance != NULL){
            delete instance;
            instance = NULL;
        } 

        printf("face-emotion instance released\n");
    }

    void fe_dumpInfos(void* pInstance)                                                    
    {
        auto instance = static_cast<Instance*>(pInstance);
        
        instance->m_faceEngine.DumpSDKInfos();
    }

    RegisterResultResponse doRegisterFaces(DBProxy& dbProxy, const std::list<fem::MyFaceInfo*>& faceInfoList)
    {
        // 注册结果
        RegisterResultResponse response;
        response.count = 0;
        response.elems = NULL;

        if(faceInfoList.empty()){
            printf("registering 0 faces...Done");
            return response;
        }

        response.count = faceInfoList.size();
        response.elems = new RegisterResult[response.count];

        // 存档到数据库
        int k = 0;
        for(auto iter = faceInfoList.begin(); iter != faceInfoList.end(); ++iter, ++k){
            const std::string& faceName = (*iter)->faceName;
            const std::string& imPath = (*iter)->imagePath;
            const auto& asfFaceInfo = (*iter)->faceInfo;
            const auto& asfFeature = (*iter)->faceFeature;
            dbProxy.SaveFaceFeature(faceName, reinterpret_cast<const char*>(asfFeature.feature), asfFeature.featureSize);
 
            RegisterResult& ret = response.elems[k];

            // 拷贝名字
            memset(&ret.name[0], 0, sizeof(ret.name));
            strncpy(&ret.name[0], faceName.c_str(), sizeof(ret.name));
            ret.name[sizeof(ret.name)-1] = '\0';

            // 拷贝原始图片路径
            memset(&ret.imagePath[0], 0, sizeof(ret.imagePath));
            strncpy(&ret.imagePath[0], imPath.c_str(), sizeof(ret.imagePath));
            ret.imagePath[sizeof(ret.imagePath)-1] = '\0';

            // 人脸框框
            ret.left = asfFaceInfo.faceRect.left;
            ret.top = asfFaceInfo.faceRect.top;
            ret.right = asfFaceInfo.faceRect.right;
            ret.bottom = asfFaceInfo.faceRect.bottom;
       }

       printf("registering %d faces...Done", faceInfoList.size());
       return response;
    }

    // 注册单张脸谱
    // @param imagePath 待识别的图片地址
    RegisterResultResponse fe_registerSingle(void* pInstance, const char* imagePath)
    {
        assert(pInstance != NULL);
        assert(imagePath != NULL);

        auto instance = static_cast<Instance*>(pInstance);
        auto& faceEngine = instance->m_faceEngine;
        auto& settings = instance->m_setting;
        auto& dbProxy = instance->m_dbProxy;
        auto& recognizer = instance->m_recognizer;
        
        printf("\nregistering face: %s...\n", imagePath);
        
        const std::string nv21Root = "./nv21.tmp.dir";

        MHandle& handle = faceEngine.GetHandle();
        std::list<fem::MyFaceInfo*> faceInfoList;
        fem::DetectFaces(handle, NULL, imagePath, faceInfoList);

        return doRegisterFaces(dbProxy, faceInfoList);
    }

    // 注册脸谱
    // @param imagePathRoot 待识别的图片地址
    // @return int 新注册的脸谱个数
    RegisterResultResponse fe_registerBatch(void* pInstance, const char* imagePathRoot)
    {
        assert(pInstance != NULL);
        assert(imagePathRoot != NULL);

        auto instance = static_cast<Instance*>(pInstance);
        auto& faceEngine = instance->m_faceEngine;
        auto& settings = instance->m_setting;
        auto& dbProxy = instance->m_dbProxy;
        auto& recognizer = instance->m_recognizer;
        
        int count = 0;

        printf("\nregistering faces in path: %s...\n", imagePathRoot);
 
        const std::string nv21Root = "./nv21.tmp.dir";

        MHandle& handle = faceEngine.GetHandle();
        std::list<fem::MyFaceInfo*> faceInfoList;
        fem::DetectFaces(handle, imagePathRoot, NULL, faceInfoList);
        
        return doRegisterFaces(dbProxy, faceInfoList);
    }

    // 释放注册结果
    void fe_releaseRegisterResult(RegisterResultResponse what)
    {
        if(what.elems != NULL){
            delete[] what.elems;
            what.elems = NULL;
        }
    }

    // 进行实际的识别分类
    RecognizeResultResponse doRecognizeFaces(Recognize& recognizer, MHandle& handle, const std::list<fem::MyFaceInfo*>& faceInfoList)
    {
        RecognizeResultResponse response;
        response.count = 0;
        response.elems = NULL;

        if(faceInfoList.empty()){
            printf("recognizing 0 faces...Done");
            return response;
        }

        response.count = faceInfoList.size();
        response.elems = new RecognizeResult[response.count];

        // 进行脸部特征匹配 
        int k = 0;
        for(auto iter = faceInfoList.begin(); iter != faceInfoList.end(); ++iter, ++k){
            const std::string& faceName = (*iter)->faceName;
            const std::string& imPath = (*iter)->imagePath;
            const auto& asfFaceInfo = (*iter)->faceInfo;
            auto& asfFeature = (*iter)->faceFeature;
 
            // 识别结果
            auto& ret = response.elems[k];

            // 拷贝图片名字
            memset(&ret.name[0], 0, sizeof(ret.name));
            strncpy(&ret.name[0], faceName.c_str(), sizeof(ret.name));
            ret.name[sizeof(ret.name)-1] = '\0';

            // 拷贝原始图片路径
            memset(&ret.imagePath[0], 0, sizeof(ret.imagePath));
            strncpy(&ret.imagePath[0], imPath.c_str(), sizeof(ret.imagePath));
            ret.imagePath[sizeof(ret.imagePath)-1] = '\0';

            // 人脸框框
            ret.left = asfFaceInfo.faceRect.left;
            ret.top = asfFaceInfo.faceRect.top;
            ret.right = asfFaceInfo.faceRect.right;
            ret.bottom = asfFaceInfo.faceRect.bottom;

           // 通过特征匹配
            Recognize::SimularType simular = recognizer.SearchSimular(handle, asfFeature);
            if(simular.second != NULL){
                // 拷贝图片名字
                memset(&ret.personName[0], 0, sizeof(ret.personName));
                strncpy(&ret.personName[0], simular.second->name.c_str(), sizeof(ret.personName));
                ret.personName[sizeof(ret.personName)-1] = '\0';

                ret.confidenceLevel = (float)simular.first;
            }else{
                ret.personName[0] = '\0';
                ret.confidenceLevel = 0.0f;
            } 
       }

       printf("recognizing %d faces...Done", faceInfoList.size());
       return response;
    }

    // 识别图片
    // @param imagePath 待识别的图片地址
    // @return 识别结果，name[0] == null 表示无法识别
    RecognizeResultResponse fe_recognizeSingle(void* pInstance, const char* imagePath)
    {
        assert(pInstance != NULL);
        assert(imagePath != NULL);

        auto instance = static_cast<Instance*>(pInstance);
        auto& faceEngine = instance->m_faceEngine;
        auto& settings = instance->m_setting;
        auto& dbProxy = instance->m_dbProxy;
        auto& recognizer = instance->m_recognizer;
        
        printf("\nrecognizing face: %s...\n", imagePath);
        
        const std::string nv21Root = "./nv21.tmp.dir";

        MHandle& handle = faceEngine.GetHandle();
        std::list<fem::MyFaceInfo*> faceInfoList;
        fem::DetectFaces(handle, NULL, imagePath, faceInfoList);
        
        return doRecognizeFaces(recognizer, handle, faceInfoList);
    }

    // 识别图片
    // @param imagePathRoot 待识别的图片地址
    // @return 识别结果，name[0] == null 表示无法识别
    RecognizeResultResponse fe_recognizeBatch(void* pInstance, const char* imagePathRoot)
    {
        assert(pInstance != NULL);
        assert(imagePathRoot != NULL);

        auto instance = static_cast<Instance*>(pInstance);
        auto& faceEngine = instance->m_faceEngine;
        auto& settings = instance->m_setting;
        auto& dbProxy = instance->m_dbProxy;
        auto& recognizer = instance->m_recognizer;
        
        printf("\nrecognizing face: %s...\n", imagePathRoot);
        
        const std::string nv21Root = "./nv21.tmp.dir";

        MHandle& handle = faceEngine.GetHandle();
        std::list<fem::MyFaceInfo*> faceInfoList;
        fem::DetectFaces(handle, imagePathRoot, NULL, faceInfoList);
        
        return doRecognizeFaces(recognizer, handle, faceInfoList);
    }

    // 释放检测结果
    void fe_releaseRecognizeResult(RecognizeResultResponse what)
    {
        if(what.elems != NULL){
            delete[] what.elems;
            what.elems = NULL;
        }
    }

}
