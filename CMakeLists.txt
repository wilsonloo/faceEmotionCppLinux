cmake_minimum_required(VERSION 3.0)		#指定cmake版本 
project(faceEmotionCppLinux)				#指定项目的名称，一般和项目的文件夹名称对应

set(CMAKE_CXX_STANDARD 11)				#设置c++标准
set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") #debug模式下开启-g选项

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# If you only include this third party in PRIVATE source files, you do not
# need to install it when your main project gets installed.
# set(JSON_Install OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.
add_subdirectory(./3rd/nlohmann_json)

#指定头文件目录
include_directories(./)	
include_directories(./include)

#指定静态和动态文件目录
link_directories(./lib)

#
# 人脸注册相关 *************************************************************
# 
#该指令的主要作用就是将指定的源文件生成链接文件，然后添加到工程中去。
#arcsoft_face生成的库文件名		[STATIC | SHARED | MODULE] 库类型
add_executable(register
        ./include/amcomdef.h
        ./include/arcsoft_face_sdk.h
		./include/asvloffscreen.h
		./include/merror.h
        ./source/faceEngine.cpp
        ./source/dbProxy.cpp
		./source/register/register.cpp)

#-fPIC	在给定的作用域内设置一个命名的属性
set_property(TARGET register PROPERTY POSITION_INDEPENDENT_CODE ON)


#链接库文件
target_link_libraries(register
        PRIVATE nlohmann_json::nlohmann_json
		arcsoft_face
		arcsoft_face_engine
        opencv_core
        opencv_highgui
        opencv_imgproc
        opencv_imgcodecs
        sqlite3
        )

# 发布
add_custom_command(TARGET register POST_BUILD
            COMMAND cp register ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/lib/libarcsoft_face.so ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/lib/libarcsoft_face_engine.so ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsqlite3.so ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/../bin/nv21.tmp.dir
        )


#
# 人脸识别相关 *************************************************************
# 
#该指令的主要作用就是将指定的源文件生成链接文件，然后添加到工程中去。
#arcsoft_face生成的库文件名		[STATIC | SHARED | MODULE] 库类型
add_executable(recognize
        ./include/amcomdef.h
        ./include/arcsoft_face_sdk.h
		./include/asvloffscreen.h
		./include/merror.h
        ./source/faceEngine.cpp
        ./source/dbProxy.cpp
		./source/recognize/recognize.cpp)

#-fPIC	在给定的作用域内设置一个命名的属性
set_property(TARGET recognize PROPERTY POSITION_INDEPENDENT_CODE ON)

#链接库文件
target_link_libraries(recognize
        PRIVATE nlohmann_json::nlohmann_json
		arcsoft_face
		arcsoft_face_engine
        opencv_core
        opencv_highgui
        opencv_imgproc
        opencv_imgcodecs
        sqlite3
        )

# 发布
add_custom_command(TARGET recognize POST_BUILD
            COMMAND cp recognize ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/lib/libarcsoft_face.so ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/lib/libarcsoft_face_engine.so ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsqlite3.so ${CMAKE_CURRENT_BINARY_DIR}/../bin
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/../bin/nv21.tmp.dir
        )

#
# 人脸识别so相关 *************************************************************
# 
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
ADD_LIBRARY(faceEmotion SHARED 
    ./include/amcomdef.h
    ./include/arcsoft_face_sdk.h
    ./include/asvloffscreen.h
    ./include/merror.h
    ./source/faceEngine.cpp
    ./source/dbProxy.cpp
    ./source/recognize/recognize.cpp
    ./source/libFaceEmotion/libFaceEmotion.cpp
    )

#链接库文件
target_link_libraries(faceEmotion
        PRIVATE nlohmann_json::nlohmann_json
		arcsoft_face
		arcsoft_face_engine
        opencv_core
        opencv_highgui
        opencv_imgproc
        opencv_imgcodecs
        sqlite3
        )


