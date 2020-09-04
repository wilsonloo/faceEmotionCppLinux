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
    isTargetDir = False
    if os.path.isdir(targetPath):
        isTargetDir = True
        #print("注册目录：", targetPath)
    elif os.path.isfile(targetPath):
        isTargetDir = False
        #print("注册图片：", targetPath)
    else:
        print(targetPath, ": is a special file(socket,FIFO,device file)")
        sys.exit(1)

    fe = instance["fe"]
    
    # 获取版本信息
    getFaceEmotionVersion = fe.fe_dumpInfos
    getFaceEmotionVersion(instance["handle"])

    if isTargetDir:
        # 目录识别
        recognizeBatch = fe.fe_recognizeBatch
        recognizeBatch.restype = (RecognizeResultResponse)
        return recognizeBatch(instance["handle"], bytes(targetPath, encoding='utf-8'))
    else:
        # 单张图片注册
        recognizeSingle = fe.fe_recognizeSingle
        recognizeSingle.restype = (RecognizeResultResponse)
        return recognizeSingle(instance["handle"], bytes(targetPath, encoding='utf-8'))
    
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

