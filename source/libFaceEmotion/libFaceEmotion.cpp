#include <string.h>
#include <stdio.h>
    
extern "C" 
{
    const char* fe_getVersion()                                                    
    {                                                                                                       return "face-emotion version: 1.0.0";
    }

    struct RecognizeResult
    {
        // 错误码（等同于arcface的错误码）
        int code; 

        // 识别到的对方名字
        char name[100];   
        
        // 识别置信等级
        float confidenceLevel;          
    };

    // 识别图片
    // @param imagePath 待识别的图片地址
    // @return 识别结果，name[0] == null 表示无法识别
    struct RecognizeResult fe_recognize(const char* imagePath)
    {
        printf("\nrecognizing %s...\n", imagePath);
        
        RecognizeResult ret;
        ret.code = 1111;
        
        memset(&ret.name[0], 0, sizeof(ret.name));
        ret.name[0] = 'a';
        ret.name[1] = 'b';
        ret.name[2] = 'c';
        ret.name[3] = '\0';

        ret.confidenceLevel = 345.678f;
        
        return ret;
    }
}
