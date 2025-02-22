#include <iostream>  
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <vector>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

#include "common.h"
#include "utils.h"
#include "data_dump.hpp"
#include "nlohmann/json.hpp"

#include "faceEngine.h"
#include "dbProxy.h"
#include "recognize.h"

using namespace std;

#define NSCALE 16 
#define FACENUM	5

Recognize::~Recognize()
{
    for(auto iter = m_persons.begin(); iter != m_persons.end(); ++iter){
        if(iter->second != NULL){
            delete iter->second;
            iter->second = NULL;
        }
    }
}

// 在全部脸谱
void Recognize::LoadAllFaces(DBProxy& dbProxy)
{
    dbProxy.LoadAllFaces(m_persons);
}

Recognize::SimularType Recognize::SearchSimular(MHandle& handle, ASF_FaceFeature& feature)
{
    MFloat maxLevel = 0;
    PersonInfo* maxPersonInfo = NULL;

    // fem::utils::data_dump(stdout, feature.feature, feature.featureSize, "AAAAAAA");

    for(auto iter = m_persons.begin(); iter != m_persons.end(); ++iter){
	    // 单人脸特征比对
        MFloat confidenceLevel;

        PersonInfo* personInfo = iter->second;
        // fem::utils::data_dump(stdout, feature.feature, feature.featureSize, std::string("person-").append(personInfo->name).c_str());

        MRESULT res = ASFFaceFeatureCompare(handle, &personInfo->feature, &feature, &confidenceLevel);
        if (res != MOK){
            printf("[Debug] ASFFaceFeatureCompare with: %s fail: %d\n", personInfo->name.c_str(), res);
        }else{ 
            printf("[Debug] ASFFaceFeatureCompare with: %s sucess: %lf\n", personInfo->name.c_str(), confidenceLevel);
            
            // 生活照相似度，要>=0.8 才算是匹配
            if(confidenceLevel >= 0.8 && confidenceLevel > maxLevel){
                maxLevel = confidenceLevel;
                maxPersonInfo = personInfo;
            }
        }
    }

    return Recognize::SimularType(maxLevel, maxPersonInfo); 
}

nlohmann::json g_setting;
DBProxy g_dbProxy;

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

    const std::string rootPath(g_setting["recognize_images_path"]);

    Recognize recognizer;
    recognizer.LoadAllFaces(g_dbProxy);

    printf("\nrecognizing images in: %s...\n", rootPath.c_str());

    MHandle& handle = faceEngine.GetHandle();
    std::list<fem::MyFaceInfo*> faceInfoList;
    fem::DetectFaces(handle, rootPath.c_str(), NULL, faceInfoList);

    // 进行人脸分类
    for(auto iter = faceInfoList.begin(); iter != faceInfoList.end(); ++iter){
        fem::MyFaceInfo* myFaceInfo = *iter;
        printf("\n**************************************************\n");
        printf("image %s\n", myFaceInfo->imagePath.c_str());
        printf("tag  : %s\n", myFaceInfo->faceName.c_str());
        printf("rect : [%d, %d, %d, %d] origin: %d\n", 
                myFaceInfo->faceInfo.faceRect.left, 
                myFaceInfo->faceInfo.faceRect.top,
                myFaceInfo->faceInfo.faceRect.right,
                myFaceInfo->faceInfo.faceRect.bottom,
                myFaceInfo->faceInfo.faceOrient);
        
        Recognize::SimularType simular = recognizer.SearchSimular(handle, myFaceInfo->faceFeature);
        if(simular.second != NULL){
            printf("maybe: %s, confidenceLevel: %.2f\n", simular.second->name.c_str(), simular.first);
        }else{
            printf("no one found.\n");
        } 
	}

    printf("recognizing nv21 images...Done\n");
    
    /*********以下三张图片均存在，图片保存在 ./bulid/images/ 文件夹下*********/
    /*	
	//可见光图像 NV21格式裸数据
	char* picPath1 = "../images/640x480_1.NV21";
	int Width1 = 640;
	int Height1 = 480;
	int Format1 = ASVL_PAF_NV21;
	MUInt8* imageData1 = (MUInt8*)malloc(Height1*Width1*3/2);
	FILE* fp1 = fopen(picPath1, "rb");
	
	//可见光图像 NV21格式裸数据
	char* picPath2 = "../images/640x480_2.NV21";
	int Width2 = 640;
	int Height2 = 480;
	int Format2 = ASVL_PAF_NV21;
	MUInt8* imageData2 = (MUInt8*)malloc(Height1*Width1*3/2);
	FILE* fp2 = fopen(picPath2, "rb");
	
	//红外图像 NV21格式裸数据
	char* picPath3 = "../images/640x480_3.NV21";
	int Width3 = 640;
	int Height3 = 480;
	int Format3 = ASVL_PAF_GRAY;
	MUInt8* imageData3 = (MUInt8*)malloc(Height2*Width2);	//只读NV21前2/3的数据为灰度数据
	FILE* fp3 = fopen(picPath3, "rb");

	if (fp1 && fp2 && fp3)
	{
		fread(imageData1, 1, Height1*Width1*3/2, fp1);	//读NV21裸数据
		fclose(fp1);
		fread(imageData2, 1, Height1*Width1*3/2, fp2);	//读NV21裸数据
		fclose(fp2);
		fread(imageData3, 1, Height3*Width3, fp3);		//读NV21前2/3的数据,用于红外活体检测
		fclose(fp3);
		
		ASVLOFFSCREEN offscreen1 = { 0 };
		ColorSpaceConversion(Width1, Height1, ASVL_PAF_NV21, imageData1, offscreen1);
		
		//第一张人脸
		ASF_MultiFaceInfo detectedFaces1 = { 0 };
		ASF_SingleFaceInfo SingleDetectedFaces = { 0 };
		ASF_FaceFeature feature1 = { 0 };
		ASF_FaceFeature copyfeature1 = { 0 };
		
        MHandle& handle = faceEngine.GetHandle();
		MRESULT res = ASFDetectFacesEx(handle, &offscreen1, &detectedFaces1);;
		if (res != MOK && detectedFaces1.faceNum > 0)
		{
			printf("%s ASFDetectFaces 1 fail: %d\n", picPath1, res);
		}
		else
		{
			SingleDetectedFaces.faceRect.left = detectedFaces1.faceRect[0].left;
			SingleDetectedFaces.faceRect.top = detectedFaces1.faceRect[0].top;
			SingleDetectedFaces.faceRect.right = detectedFaces1.faceRect[0].right;
			SingleDetectedFaces.faceRect.bottom = detectedFaces1.faceRect[0].bottom;
			SingleDetectedFaces.faceOrient = detectedFaces1.faceOrient[0];
			
			// 单人脸特征提取
			res = ASFFaceFeatureExtractEx(handle, &offscreen1, &SingleDetectedFaces, &feature1);
			if (res != MOK)
			{
				printf("%s ASFFaceFeatureExtractEx 1 fail: %d\n", picPath1, res);
			}
			else
			{
				//拷贝feature，否则第二次进行特征提取，会覆盖第一次特征提取的数据，导致比对的结果为1
				copyfeature1.featureSize = feature1.featureSize;
				copyfeature1.feature = (MByte *)malloc(feature1.featureSize);
				memset(copyfeature1.feature, 0, feature1.featureSize);
				memcpy(copyfeature1.feature, feature1.feature, feature1.featureSize);
			}
		}
		
		//第二张人脸
		ASVLOFFSCREEN offscreen2 = { 0 };
		ColorSpaceConversion(Width2, Height2, ASVL_PAF_NV21, imageData2, offscreen2);
		
		ASF_MultiFaceInfo detectedFaces2 = { 0 };
		ASF_FaceFeature feature2 = { 0 };
		
		res = ASFDetectFacesEx(handle, &offscreen2, &detectedFaces2);
		if (res != MOK && detectedFaces2.faceNum > 0)
		{
			printf("%s ASFDetectFacesEx 2 fail: %d\n", picPath2, res);
		}
		else
		{
			SingleDetectedFaces.faceRect.left = detectedFaces2.faceRect[0].left;
			SingleDetectedFaces.faceRect.top = detectedFaces2.faceRect[0].top;
			SingleDetectedFaces.faceRect.right = detectedFaces2.faceRect[0].right;
			SingleDetectedFaces.faceRect.bottom = detectedFaces2.faceRect[0].bottom;
			SingleDetectedFaces.faceOrient = detectedFaces2.faceOrient[0];
			
			res = ASFFaceFeatureExtractEx(handle, &offscreen2, &SingleDetectedFaces, &feature2);
			if (res != MOK)
				printf("%s ASFFaceFeatureExtractEx 2 fail: %d\n", picPath2, res);
			else
				printf("%s ASFFaceFeatureExtractEx 2 sucess: %d\n", picPath2, res);
		}

		// 单人脸特征比对
		MFloat confidenceLevel;
		res = ASFFaceFeatureCompare(handle, &copyfeature1, &feature2, &confidenceLevel);
		if (res != MOK)
			printf("ASFFaceFeatureCompare fail: %d\n", res);
		else
			printf("ASFFaceFeatureCompare sucess: %lf\n", confidenceLevel);
		

		printf("\n************* Face Process *****************\n");
		//设置活体置信度 SDK内部默认值为 IR：0.7  RGB：0.5（无特殊需要，可以不设置）
		ASF_LivenessThreshold threshold = { 0 };
		threshold.thresholdmodel_BGR = 0.5;
		threshold.thresholdmodel_IR = 0.7;
		res = ASFSetLivenessParam(handle, &threshold);
		if (res != MOK)
			printf("ASFSetLivenessParam fail: %d\n", res);
		else
			printf("RGB Threshold: %f\nIR Threshold: %f\n", threshold.thresholdmodel_BGR, threshold.thresholdmodel_IR);

		// 人脸信息检测
		MInt32 processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
		res = ASFProcessEx(handle, &offscreen2, &detectedFaces2, processMask);
		if (res != MOK)
			printf("ASFProcessEx fail: %d\n", res);
		else
			printf("ASFProcessEx sucess: %d\n", res);

		// 获取年龄
		ASF_AgeInfo ageInfo = { 0 };
		res = ASFGetAge(handle, &ageInfo);
		if (res != MOK)
			printf("%s ASFGetAge fail: %d\n", picPath2, res);
		else
			printf("%s First face age: %d\n", picPath2, ageInfo.ageArray[0]);

		// 获取性别
		ASF_GenderInfo genderInfo = { 0 };
		res = ASFGetGender(handle, &genderInfo);
		if (res != MOK)
			printf("%s ASFGetGender fail: %d\n", picPath2, res);
		else
			printf("%s First face gender: %d\n", picPath2, genderInfo.genderArray[0]);

		// 获取3D角度
		ASF_Face3DAngle angleInfo = { 0 };
		res = ASFGetFace3DAngle(handle, &angleInfo);
		if (res != MOK)
			printf("%s ASFGetFace3DAngle fail: %d\n", picPath2, res);
		else
			printf("%s First face 3dAngle: roll: %lf yaw: %lf pitch: %lf\n", picPath2, angleInfo.roll[0], angleInfo.yaw[0], angleInfo.pitch[0]);
		
		//获取活体信息
		ASF_LivenessInfo rgbLivenessInfo = { 0 };
		res = ASFGetLivenessScore(handle, &rgbLivenessInfo);
		if (res != MOK)
			printf("ASFGetLivenessScore fail: %d\n", res);
		else
			printf("ASFGetLivenessScore sucess: %d\n", rgbLivenessInfo.isLive[0]);
		
		
		printf("\n**********IR LIVENESS*************\n");
		
		//第二张人脸
		ASVLOFFSCREEN offscreen3 = { 0 };
		ColorSpaceConversion(Width3, Height3, ASVL_PAF_GRAY, imageData3, offscreen3);
		
		ASF_MultiFaceInfo detectedFaces3 = { 0 };
		res = ASFDetectFacesEx(handle, &offscreen3, &detectedFaces3);
		if (res != MOK)
			printf("ASFDetectFacesEx fail: %d\n", res);
		else
			printf("Face num: %d\n", detectedFaces3.faceNum);
		
		//IR图像活体检测
		MInt32 processIRMask = ASF_IR_LIVENESS;
		res = ASFProcessEx_IR(handle, &offscreen3, &detectedFaces3, processIRMask);
		if (res != MOK)
			printf("ASFProcessEx_IR fail: %d\n", res);
		else
			printf("ASFProcessEx_IR sucess: %d\n", res);
		
		//获取IR活体信息
		ASF_LivenessInfo irLivenessInfo = { 0 };
		res = ASFGetLivenessScore_IR(handle, &irLivenessInfo);
		if (res != MOK)
			printf("ASFGetLivenessScore_IR fail: %d\n", res);
		else
			printf("IR Liveness: %d\n", irLivenessInfo.isLive[0]);
		
		//释放内存
		SafeFree(copyfeature1.feature);
		SafeArrayDelete(imageData1);
		SafeArrayDelete(imageData2);
		SafeArrayDelete(imageData3);
		

		//反初始化
		res = ASFUninitEngine(handle);
		if (res != MOK)
			printf("ASFUninitEngine fail: %d\n", res);
		else
			printf("ASFUninitEngine sucess: %d\n", res);
	}
	else
	{
		printf("No pictures found.\n");
	}
    */

	getchar();
    return 0;
}

