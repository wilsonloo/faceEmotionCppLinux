import os
import sys
sys.path.append('/usr/local/lib64')
sys.path.append('/home/wilson/faceEmotionCppLinux/lib')

import ctypes
from ctypes import *

# 识别结果
class RecognizeResult(Structure):
    _fields_ = [
        ("name", ctypes.c_char * 100),
        ("imagePath", ctypes.c_char * 1024),
        ("left", ctypes.c_int),
        ("top", ctypes.c_int),
        ("right", ctypes.c_int),
        ("bottom", ctypes.c_int),
        ("personName", ctypes.c_char * 100),
        ("confidenceLevel", ctypes.c_float)
    ]
    
class RecognizeResultResponse(Structure):
    _fields_ = [
        ("count", ctypes.c_int),
        ("elems", POINTER(RecognizeResult))
    ]

# 识别函数入口
def recognize(instance, targetPath):
    # 检验解码目录
    if not os.path.exists('./nv21.tmp.dir'):
        os.makedirs('./nv21.tmp.dir')

    # 根据对象是单文件还是路径，进行区分注册
    targetDirPath = None
    targetImagePath = None
    if os.path.isdir(targetPath):
        targetDirPath = bytes(targetPath, encoding='utf-8')
    elif os.path.isfile(targetPath):
        targetImagePath = bytes(targetPath, encoding='utf-8')
    else:
        print(targetPath, ": is a special file(socket,FIFO,device file)")
        sys.exit(1)

    fe = instance["fe"]
    handle = instance["handle"]
    
    # 获取版本信息
    getFaceEmotionVersion = fe.fe_dumpInfos
    getFaceEmotionVersion(handle)

    # 获取c++的识别函数
    recognizeFunc = fe.fe_recognize
    recognizeFunc.restype = (RecognizeResultResponse)
    return recognizeFunc(handle, targetDirPath, targetImagePath)

# ###############################################################
# 释放注册结果    
def releaseRecognizeResponse(instance, response):
    fe = cdll.LoadLibrary("libfaceEmotion.so")
    releaser = fe.fe_releaseRecognizeResult
    releaser(response)
    response = None

def createInstance():
    # 加载内部库
    fe = cdll.LoadLibrary("libfaceEmotion.so") 
    # 初始化实例
    initor = fe.fe_init
    initor.restype = ctypes.c_void_p
    handle = initor()
    return {"fe":fe, "handle":handle}

def releaseInstance(instance):
    if instance != None:
        releaser = instance["fe"].fe_release
        releaser(instance["handle"])
        instance = None

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python recognize.py <图片或目录>")
        sys.exit(1)
        
    instance = createInstance()
    # sys.argv[0])           #sys.argv[0] 类似于shell中的$0,但不是脚本名称，而是脚本的路径   
    targetPath = sys.argv[1] #sys.argv[1] 表示传入的第一个参数, 注册对象路径
    response = recognize(instance, targetPath) 
    releaseRecognizeResponse(instance, reaponse)

