#include <string.h>
#include <stdio.h>
#include "recognize.h"
#include "faceEngine.h"
    
extern "C" 
{
    // 引擎实例
    struct Instance 
    {
        // 基本引擎
        FaceEngine faceEngine;

        // 识别器
        Recognize m_recognizer;
    };

    struct RecognizeResult
    {
        // 错误码（等同于arcface的错误码）
        int code; 

        // 识别到的对方名字
        char name[100];   
        
        // 识别置信等级
        float confidenceLevel;          
    };

    void* fe_init()
    {
        Instance* instance = new Instance();
        instance->faceEngine.Init();

        return instance;
    }

    void fe_dumpInfos(void* pInstance)                                                    
    {
        auto instance = static_cast<Instance*>(pInstance);
        
        instance->faceEngine.DumpSDKInfos();
    }

    // 识别图片
    // @param imagePath 待识别的图片地址
    // @return 识别结果，name[0] == null 表示无法识别
    struct RecognizeResult fe_recognize(void* pInstance, const char* imagePath)
    {
        auto instance = static_cast<Instance*>(pInstance);

        printf("\nrecognizing %s...\n", imagePath);
        
        RecognizeResult ret;
        /* 测试
        ret.code = 1111;
        
        memset(&ret.name[0], 0, sizeof(ret.name));
        ret.name[0] = 'a';
        ret.name[1] = 'b';
        ret.name[2] = 'c';
        ret.name[3] = '\0';

        ret.confidenceLevel = 345.678f;
        */



        return ret;
    }
}
