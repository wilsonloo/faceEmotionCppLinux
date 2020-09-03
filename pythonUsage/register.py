import os
import sys
sys.path.append('/usr/local/lib64')
sys.path.append('/home/wilson/faceEmotionCppLinux/lib')

import ctypes
from ctypes import *

if len(sys.argv) < 2:
    print("Usage: python register.py <图片或目录>")
    sys.exit(1)

# sys.argv[0])           #sys.argv[0] 类似于shell中的$0,但不是脚本名称，而是脚本的路径   
targetPath = sys.argv[1] #sys.argv[1] 表示传入的第一个参数, 注册对象路径

# 根据对象是单文件还是路径，进行区分注册
isTargetDir = False
if os.path.isdir(targetPath):
    isTargetDir = True
    print("注册目录：", targetPath)
elif os.path.isfile(targetPath):
    isTargetDir = False
    print("注册图片：", targetPath)
else:
    print(targetPath, "it's a special file(socket,FIFO,device file)")
    sys.exit(1)

# 注册结果
class RegisterResult(Structure):
    _fields_ = [
        ("name", ctypes.c_char * 100),
        ("imagePath", ctypes.c_char * 1024)
    ]


try:
    # 加载内部库
    fe = cdll.LoadLibrary("libfaceEmotion.so")   
    
    # 初始化实例
    initor = fe.fe_init
    initor.restype = ctypes.c_void_p
    instance = initor()

    # 获取版本信息
    getFaceEmotionVersion = fe.fe_dumpInfos
    getFaceEmotionVersion(instance)

    if isTargetDir:
        # 目录注册
        print("unimpleted yes")
        sys.exit(2)
    else:
        # 单张图片注册
        registerSingle = fe.fe_registerSingle
        registerSingle.restype = (RegisterResult)
        ret = registerSingle(bytes(targetPath, encoding='utf-8'))
        print("register result:")
        print("  ", ret.name)
        print("  ", ret.imagePath)

finally:
    if instance != None:
        releaser = fe.fe_release
        releaser(instance)
        instance = None

