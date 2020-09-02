#ifndef _FACE_EMOTION_DB_PROXY_H_
#define _FACE_EMOTION_DB_PROXY_H_

#include <string>
#include "sqlite3.h"

class DBProxy
{
private:
    std::string m_dbPath = "";
    sqlite3*    m_db     = NULL;

public:
    bool Init(const std::string& dbPath);

    // 保存脸谱
    void SaveFaceFeature(const std::string& faceName,  const char* feature, int featureSize);

private:
    bool fixTableFace();
};

#endif
