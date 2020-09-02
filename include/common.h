#ifndef _FACE_EMOTION_COMMON_H_
#define _FACE_EMOTION_COMMON_H_

namespace fem
{
    // 加载所有带注册的图片,并转为nv21格式
    void ConvertRGB2NV21Images(const std::string& rootPath, const std::string& targetPath)
    {
        printf("converting RGB images to NV21 in %s...\n", rootPath.c_str());

        // 获取目录下的所有jpg文件
        std::list<std::string> imagePathList;
        const std::string suffix = "png";
        fem::utils::getFilePathsInDirectory(rootPath, suffix, imagePathList);

        // 对每个jpg进行nv21转码
        for(const std::string& imagePath : imagePathList){
            printf("\tconverting RGB image %s\n", imagePath.c_str());

            // 将jpg改为nv21
            std::string imageName = fem::utils::getFileName(imagePath);
            std::string nv21Path = targetPath;
            nv21Path.append(imageName.substr(0, imageName.length()-suffix.length()));
            nv21Path.append("nv21");

            fem::utils::opencvRGB2NV21(imagePath, nv21Path);
        }

        printf("\tconverting RGB images to NV21...Done\n");
    }
}

#endif
