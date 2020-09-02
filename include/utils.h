#ifndef _FACE_EMOTION_UTILS_H__
#define _FACE_EMOTION_UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <string.h>
#include <algorithm>  
#include <fstream>
#include <dirent.h>

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"

#include <opencv2/opencv.hpp>

namespace fem
{
    namespace utils
    {
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

        //时间戳转换为日期格式
        static void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
        {
            time_t tTimeStamp = atoll(timeStamp);
            struct tm* pTm = gmtime(&tTimeStamp);
            strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
        }

        static void opencvRGB2NV21(const std::string& infile, const char* outfile)
        {
            cv::Mat Img = cv::imread(infile);
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
            }
        }

        // 获取文件的路径(是否全路径，取决与参数自己，本接口只是去除了文件类型)
        static std::string getFilePath(const std::string& fileFullName)
        {
            std::string directory;
            const size_t last_slash_index = fileFullName.rfind('\\');
            if(std::string::npos != last_slash_index){
                std::string directory = fileFullName.substr(0, last_slash_index);
                return directory;
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
                    int index = path.size() - suffix.size();
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
