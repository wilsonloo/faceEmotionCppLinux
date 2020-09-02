#ifndef _FACE_EMOTION_RECOGNIZE_H_
#define _FACE_EMOTION_RECOGNIZE_H_

#include <map>
#include <string>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

#include "personInfo.h"
#include "dbProxy.h"

class Recognize
{
private:
    PersonMapType m_persons;

public:
    virtual ~Recognize();

    // 加载全部脸谱
    void LoadAllFaces(DBProxy& dbProxy);

    // 识别指定路径的图片
    void RecognizeImages(const std::string& path);
};

#endif
