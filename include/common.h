#ifndef _FACE_EMOTION_COMMON_H_
#define _FACE_EMOTION_COMMON_H_

#include <list>
#include <string>
#include "utils.h"

namespace fem
{
    // 加载所有带注册的图片,并转为nv21格式
    static void ConvertRGB2NV21Images(const char* rootPath, 
            const char* singlePath, 
            const std::string& targetPath, 
            const int targetWidth, 
            const int targetHeight,
            /*out*/std::list<std::string>* outImagePathList = NULL, 
           /*out*/std::list<std::string>* outNV21PathList = NULL)
    {
        // 获取目录下的所有jpg文件
        const std::list<std::string> suffixList = {"png", "jpg"};
        
        // 预处理图片列表
        std::list<std::string> imagePathList;

        // 预处理单张图片
        if(singlePath != NULL)
        {
            auto fileType = fem::utils::getFileType(singlePath);
            for(const std::string& suffix : suffixList){
                if(fileType == suffix){
                    imagePathList.push_back(singlePath);
                    break; 
                } 
            }
        }

        // 预处理目录里的所有文件
        if(rootPath != NULL){
            for(const std::string& suffix : suffixList){
                fem::utils::getFilePathsInDirectory(rootPath, suffix, imagePathList);
            }
        }

        // 对每个jpg进行nv21转码
        for(const std::string& imagePath : imagePathList){
            // 将jpg改为nv21
            auto imageName = fem::utils::getFileName(imagePath);
            auto imageType = fem::utils::getFileType(imagePath);

            auto nv21Path = targetPath + "/";
            nv21Path.append(imageName.substr(0, imageName.length()-imageType.length()));
            nv21Path.append("nv21");

            fem::utils::opencvRGB2NV21(imagePath, targetWidth, targetHeight, nv21Path);

            // 进行文件返回
            if(outImagePathList != NULL){
                outImagePathList->push_back(imagePath);
            }
            if(outNV21PathList != NULL){
                outNV21PathList->push_back(nv21Path);
            }
        }
    }
        
    //图像颜色格式转换
    static int ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen)
    {
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = width;
        offscreen.i32Height = height;
        
        switch (offscreen.u32PixelArrayFormat)
        {
        case ASVL_PAF_RGB24_B8G8R8:
            offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
            offscreen.ppu8Plane[0] = imgData;
            break;
        case ASVL_PAF_I420:
            offscreen.pi32Pitch[0] = width;
            offscreen.pi32Pitch[1] = width >> 1;
            offscreen.pi32Pitch[2] = width >> 1;
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
            offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
            break;
        case ASVL_PAF_NV12:
        case ASVL_PAF_NV21:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
            break;
        case ASVL_PAF_YUYV:
        case ASVL_PAF_DEPTH_U16:
            offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
            offscreen.ppu8Plane[0] = imgData;
            break;
        case ASVL_PAF_GRAY:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.ppu8Plane[0] = imgData;
            break;
        default:
            return 0;
        }
        return 1;
    }

    struct MyFaceInfo
    {
        // 脸名
        std::string faceName;

        // 原始图片名
        std::string imagePath;

        // 脸部边框
        ASF_SingleFaceInfo faceInfo;

        // 脸部特征
        ASF_FaceFeature faceFeature;

        virtual ~MyFaceInfo()
        {
            if(faceFeature.feature != NULL){
                free(faceFeature.feature);
                faceFeature.feature = NULL;
            }

            faceFeature.featureSize = 0;
        }
    };

    static void DetectFaces(MHandle& handle, 
            const char* imageRootPath, 
            const char* imageSinglePath, 
            /*out*/std::list<MyFaceInfo*>& faceInfoList)
    {
        // 加载nv21 文件
        const int WIDTH = 640;
        const int HEIGHT = 480;
        const int FORMAT = ASVL_PAF_NV21;
        const std::string nv21Root = "./nv21.tmp.dir";

        // 将jpg图片转成nv21 文件
        printf("convering RGB iamge to nv21 files in %s...\n", nv21Root.c_str());
        std::list<std::string> imagePathList;
        std::list<std::string> nv21PathList;
        ConvertRGB2NV21Images(imageRootPath, imageSinglePath, nv21Root, WIDTH, HEIGHT, &imagePathList, &nv21PathList);
        printf("convering [%d] images Done\n", imagePathList.size());

        // 对每个nv21文件进行人脸检测
        auto nv21PathIter = nv21PathList.begin();
        auto imagePathIter = imagePathList.begin();
        for(; nv21PathIter != nv21PathList.end(); nv21PathIter++, imagePathIter++){
            auto nv21Path = *nv21PathIter;
            auto imagePath = *imagePathIter;

            // 脸的名字，没有文件类型名
            std::string faceName = fem::utils::getFileName(nv21Path);
            faceName = faceName.substr(0, faceName.length()-5);

            MUInt8* imageData = (MUInt8*)malloc(HEIGHT*WIDTH*3/2);
            FILE* filePtr = fopen(nv21Path.c_str(), "rb"); 
            if(filePtr != NULL){
                const auto& scopeGuard = fem::utils::makeScopeGuard([&](){
                    fclose(filePtr);
                });

                //读取nv21 裸数据
                fread(imageData, 1, HEIGHT*WIDTH*3/2, filePtr);
            
                ASVLOFFSCREEN offscreen = { 0 };
                ColorSpaceConversion(WIDTH, HEIGHT, ASVL_PAF_NV21, imageData, offscreen);
                
                // 检测人脸信息
                ASF_MultiFaceInfo detectedFaces = { 0 };
                
                MRESULT res = ASFDetectFacesEx(handle, &offscreen, &detectedFaces);;
                if (res != MOK && detectedFaces.faceNum > 0)
                {
                    printf("\tASFDetectFaces detect face:%s failed: %d\n", faceName.c_str(), res);
                }
                else
                {
                    printf("\tASFDetectFaces detect face:%s ok\n", faceName.c_str());

                    // 我的脸部信息
                    MyFaceInfo* myFaceInfo = new MyFaceInfo();
                    myFaceInfo->faceName = faceName;
                    myFaceInfo->imagePath = imagePath;

                    myFaceInfo->faceInfo.faceRect.left = detectedFaces.faceRect[0].left;
                    myFaceInfo->faceInfo.faceRect.top = detectedFaces.faceRect[0].top;
                    myFaceInfo->faceInfo.faceRect.right = detectedFaces.faceRect[0].right;
                    myFaceInfo->faceInfo.faceRect.bottom = detectedFaces.faceRect[0].bottom;
                    myFaceInfo->faceInfo.faceOrient = detectedFaces.faceOrient[0];
                    
                    // 单人脸特征提取
                    // 脸部特征
                    ASF_FaceFeature faceFeatureTemp;
                    res = ASFFaceFeatureExtractEx(handle, &offscreen, &myFaceInfo->faceInfo, &faceFeatureTemp);
                    if (res != MOK)
                    {
                        printf("\tASFFaceFeatureExtractEx extra face:%s feature fail: %d\n", faceName.c_str(), res);
                    }
                    else
                    {
                        //拷贝feature，否则第二次进行特征提取，会覆盖第一次特征提取的数据，导致比对的结果为1
                        myFaceInfo->faceFeature.featureSize = faceFeatureTemp.featureSize;
                        myFaceInfo->faceFeature.feature = (MByte *)malloc(faceFeatureTemp.featureSize);
                        memset(myFaceInfo->faceFeature.feature, 0, faceFeatureTemp.featureSize);
                        memcpy(myFaceInfo->faceFeature.feature, faceFeatureTemp.feature, faceFeatureTemp.featureSize);
                        faceInfoList.emplace_back(myFaceInfo);
                    }
                }
            }else{
                printf("\tprocessing %s: file not found!\n", nv21Path.c_str());
            }
        }
    }
}

#endif
