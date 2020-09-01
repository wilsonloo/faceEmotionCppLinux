#ifndef _FACE_EMOTION_FACE_ENGINE_H_
#define _FACE_EMOTION_FACE_ENGINE_H_

#include "amcomdef.h"

class FaceEngine
{
private:
    MHandle m_handle = NULL;

public:
    MHandle& GetHandle(){
        return m_handle;
    }

public:
    void DumpSDKInfos();
    void DumpActivationInfos(char* appId, char* sdkKey, char* activationKey);

    void Init();
};

#endif
