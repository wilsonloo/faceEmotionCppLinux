#ifndef _FACE_EMOTION_UTILS_H__
#define _FACE_EMOTION_UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <string.h>
#include <algorithm>  
#include <fstream>
#include <memory>
#include <dirent.h>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

#include <opencv2/opencv.hpp>

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; } 
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; } 

namespace fem
{
    namespace utils
    {
 
        /// <summary>
        /// 获取图片缩放比例,使至少一个轴与对方同等长度
        /// </summary>
        /// <param name="oldWidth">原图片宽</param>
        /// <param name="oldHeigt">原图片高</param>
        /// <param name="newWidth">目标图片宽</param>
        /// <param name="newHeight">目标图片高</param>
        /// <returns>float</returns>
        static float getWidthAndHeightScale(int oldWidth, int oldHeigt, int newWidth, int newHeight)
        {
            //按比例缩放           
            float scaleRate = 0.0f;
            if (oldWidth >= newWidth && oldHeigt >= newHeight)
            {
                int widthDis = oldWidth - newWidth;
                int heightDis = oldHeigt - newHeight;
                if (widthDis > heightDis)
                {
                    scaleRate = newWidth * 1.0f / oldWidth;
                }
                else
                {
                    scaleRate = newHeight * 1.0f / oldHeigt;
                }
            }
            else if (oldWidth >= newWidth && oldHeigt < newHeight)
            {
                scaleRate = newWidth * 1.0f / oldWidth;
            }
            else if (oldWidth < newWidth && oldHeigt >= newHeight)
            {
                scaleRate = newHeight * 1.0f / oldHeigt;
            }
            else
            {
                int widthDis = newWidth - oldWidth;
                int heightDis = newHeight - oldHeigt;
                if (widthDis > heightDis)
                {
                    scaleRate = newHeight * 1.0f / oldHeigt;
                }
                else
                {
                    scaleRate = newWidth * 1.0f / oldWidth;
                }
            }
            return scaleRate;
        }

        // ScopeGuard ===================================
        template< typename Lambda >
        class ScopeGuard
        {
        private:
            mutable bool committed;
            Lambda rollbackLambda; 

        public:
            ScopeGuard( const Lambda& _l) 
                : committed(false) 
                , rollbackLambda(_l) 
            {}

            template< typename AdquireLambda >
            ScopeGuard( const AdquireLambda& _al , const Lambda& _l) 
                : committed(false) 
                , rollbackLambda(_l)
            {
                _al();
            }

            ~ScopeGuard()
            {
                if (!committed)
                    rollbackLambda();
            }

            inline void commit() const 
            { 
                committed = true; 
            }

        };

        template< typename aLambda , typename rLambda>
        const ScopeGuard< rLambda >& makeScopeGuard( const aLambda& _a , const rLambda& _r)
        {
            return ScopeGuard< rLambda >( _a , _r );

        }

        template<typename rLambda>
        const ScopeGuard< rLambda > makeScopeGuard(const rLambda& _r)
        {
            return ScopeGuard< rLambda >(_r );
        }
        // ScopeGuard end ===================================
        
        template<typename ... Args>
        std::string string_format(const std::string& format, Args ... args){
            size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);  // Extra space for \0
            // unique_ptr<char[]> buf(new char[size]);
            char bytes[size];
            snprintf(bytes, size, format.c_str(), args ...);
            return std::string(bytes);
        }


        //时间戳转换为日期格式
        static void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
        {
            time_t tTimeStamp = atoll(timeStamp);
            struct tm* pTm = gmtime(&tTimeStamp);
            strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
        }

        static cv::Mat resizeMatImage(cv::Mat pSrc, double dScale)
        {
            cv::Size sSize = cv::Size(pSrc.cols*dScale, pSrc.rows*dScale);
            cv::Mat pDes(sSize, CV_32S);
            cv::resize(pSrc, pDes, sSize);
            return pDes;
        }

        static cv::Mat checkWidthAndHeight(cv::Mat img,const int targetWidth,const int targetHeight) 
        {
            if(img.cols == targetWidth && img.rows == targetHeight){
                return img;
            } 

            double scale = getWidthAndHeightScale(img.cols, img.rows, targetWidth, targetHeight);
            return resizeMatImage(img, scale);
        }

        static void opencvRGB2NV21(const std::string& infile, 
                const int targetWidth, const int targetHeight, const std::string& outfilePath)
        {
            const char* outfile = outfilePath.c_str();

            cv::Mat Img = cv::imread(infile);
            Img = checkWidthAndHeight(Img, targetWidth, targetWidth);

            int buflen = (int)(Img.rows * Img.cols * 3 / 2);
            unsigned char* pYuvBuf = new unsigned char[buflen];

            cv::Mat OpencvYUV;
            FILE* fout = fopen(outfile, "wb");
            if(fout != NULL){
                const auto& scopeGuard = makeScopeGuard([&](){
                    fclose(fout);
                });

                cvtColor(Img, OpencvYUV, CV_BGR2YUV_YV12);
                memcpy(pYuvBuf, OpencvYUV.data, buflen*sizeof(unsigned char));
                fwrite(pYuvBuf, buflen*sizeof(unsigned char), 1, fout);
            }else{
                printf("failed to find out file:%s\n", outfile);
            }
        }

        // 获取文件的路径(是否全路径，取决与参数自己，本接口只是去除了文件类型)
        static std::string getFilePath(const std::string& fileFullName)
        {
            std::string directory;
            const size_t last_slash_index = fileFullName.rfind('/');
            if(std::string::npos != last_slash_index){
                std::string directory = fileFullName.substr(0, last_slash_index);
                return directory;
            }

            return std::string(fileFullName);
        }

        // 获取文件名，包含文件类型
        static std::string getFileName(const std::string& fileFullName)
        {
            const size_t last_slash_index = fileFullName.rfind('/');
            if(std::string::npos != last_slash_index){
                return fileFullName.substr(last_slash_index+1);
            } 

            return std::string(fileFullName);
        }

        // 获取文件类型
        static std::string getFileType(const std::string& fileFullName)
        {
            const size_t last_slash_index = fileFullName.rfind('.');
            if(std::string::npos != last_slash_index){
                return fileFullName.substr(last_slash_index+1);
            } 

            return std::string(fileFullName);
        }


        // 遍历目录下的指定类型文件
        static void getFilePathsInDirectory(const std::string& path, const std::string& suffix, std::list<std::string>& list)
        {
            DIR* dir = opendir(path.c_str());
            if(dir != NULL){
                const auto& scopeGuard = makeScopeGuard([&](){
                    closedir(dir);
                });

                dirent* p = NULL;
                while((p = readdir(dir)) != NULL){
                    // 忽略 .. 和 . 目录
                    if(p->d_name[0] == '.'){
                        continue;
                    }

                    const std::string dName(p->d_name);

                    // 按照指定后缀字符串，截取原本路径的尾部
                    int index = dName.length() - suffix.length() - 1;
                    if(index <= 0){
                        continue;
                    }

                    // 获取小写后缀 
                    std::string tailLower = dName.substr(index);
                    std::transform(tailLower.begin(), tailLower.end(), tailLower.begin(), ::tolower);

                    std::string dotSuffix(".");
                    dotSuffix.append(suffix);
                    if(tailLower != dotSuffix){
                        continue;
                    }

                    std::string fullName(path + "/" + dName);
                    list.push_back(fullName);
                }
            }
        }        
    }
}

#endif
