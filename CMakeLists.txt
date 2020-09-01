cmake_minimum_required(VERSION 3.0)		#指定cmake版本 
project(faceEmotionCppLinux)				#指定项目的名称，一般和项目的文件夹名称对应

set(CMAKE_CXX_STANDARD 11)				#设置c++标准

#指定头文件目录
include_directories(./)	
include_directories(./include)

#指定静态和动态文件目录
link_directories(./lib)

#该指令的主要作用就是将指定的源文件生成链接文件，然后添加到工程中去。
#arcsoft_face生成的库文件名		[STATIC | SHARED | MODULE] 库类型
add_executable(register
        ./include/amcomdef.h
        ./include/arcsoft_face_sdk.h
		./include/asvloffscreen.h
		./include/merror.h
		./source/register/register.cpp)

#-fPIC	在给定的作用域内设置一个命名的属性
set_property(TARGET register PROPERTY POSITION_INDEPENDENT_CODE ON)


#链接库文件
target_link_libraries(register
		arcsoft_face
		arcsoft_face_engine
        )
