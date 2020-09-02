#include <iostream>  
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fstream>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

#include "common.h"
#include "utils.h"
#include "nlohmann/json.hpp"

#include "faceEngine.h"
#include "dbProxy.h"

using namespace std;

#define NSCALE 16 
#define FACENUM	5

nlohmann::json g_setting;
DBProxy g_dbProxy;

// todo 写入到sqlite数据库
void SaveFaceFeature(const std::string& faceName, const ASF_FaceFeature& feature)
{
    g_dbProxy.SaveFaceFeature(faceName, reinterpret_cast<const char*>(feature.feature), feature.featureSize);
}

int main()
{
    std::ifstream fin("setting.json");
    fin >> g_setting;
    fin.close();

    FaceEngine faceEngine;
    faceEngine.Init();
    faceEngine.DumpSDKInfos();

    const std::string appId(g_setting["app_id"]);
    const std::string sdkKey(g_setting["sdk_key"]);
    faceEngine.DumpActivationInfos(const_cast<char*>(appId.c_str()), const_cast<char*>(sdkKey.c_str()), NULL);

    // 初始化脸谱数据库
    bool ok = g_dbProxy.Init(g_setting["db_path"]);
    if(!ok){
        exit(1);
    }

    // 加载所有带注册的图片,并转为nv21格式
    const std::string imageRootPath(g_setting["register_images_path"]);
    const std::string nv21Root = "./nv21.tmp.dir";

    MHandle& handle = faceEngine.GetHandle();
    std::list<MyFaceInfo> faceInfoList;
    fem::DetectFaces(handle, imageRootPath, faceInfoList);

    // 存档到数据库
    for(auto iter = faceInfoList.begin(); iter != faceInfoList.end(); ++iter){
        SaveFaceFeature(iter->faceName, iter->faceInfo); 
    }

    return 0;
}

