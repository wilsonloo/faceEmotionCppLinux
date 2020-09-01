#include <stdio.h>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

#include "common.h"
#include "faceEngine.h"

#define NSCALE 16 
#define FACENUM 5

void FaceEngine::DumpSDKInfos()
{
    printf("\n*************** ArcFaceFace SDK Info *****************\n");
    MRESULT res = MOK;
    ASF_ActiveFileInfo activeFileInfo = { 0 };
    res = ASFGetActiveFileInfo(&activeFileInfo);
    if (res != MOK)
    {
        printf("ASFGetActiveFileInfo fail: %d\n", res);
    }
    else
    {
        //这里仅获取了有效期时间，还需要其他信息直接打印即可
        char startDateTime[32];
        fem::timestampToTime(activeFileInfo.startTime, startDateTime, 32);
        printf("startTime: %s\n", startDateTime);
        
        char endDateTime[32];
        fem::timestampToTime(activeFileInfo.endTime, endDateTime, 32);
        printf("endTime: %s\n", endDateTime);
    }

    //SDK版本信息
    const ASF_VERSION version = ASFGetVersion();
    printf("\nVersion:%s\n", version.Version);
    printf("BuildDate:%s\n", version.BuildDate);
    printf("CopyRight:%s\n", version.CopyRight);
}

void FaceEngine::DumpActivationInfos(char* appId, char* sdkKey, char* activationKey)
{
    printf("\n************* Activation Infos *****************\n");

    MRESULT res = ASFOnlineActivation(appId, sdkKey);
    if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
        printf("ASFOnlineActivation fail: %d\n", res);
    else
        printf("ASFOnlineActivation sucess: %d\n", res);
}

void FaceEngine::Init()
{
    //初始化引擎
    MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS | ASF_IR_LIVENESS;
    MRESULT res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, NSCALE, FACENUM, mask, &m_handle);    

    if (res != MOK)
        printf("ASFInitEngine fail: %d\n", res);
    else
        printf("ASFInitEngine sucess: %d\n", res);
}
