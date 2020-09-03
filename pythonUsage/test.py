import sys
sys.path.append('/usr/local/lib64')
sys.path.append('/home/wilson/faceEmotionCppLinux/lib')

import ctypes
from ctypes import *

# 加载内部库
fe = cdll.LoadLibrary("libfaceEmotion.so")   

# 初始化实例
initor = fe.fe_init
initor.restype = ctypes.c_void_p
instance = initor()

# 获取版本信息
getFaceEmotionVersion = fe.fe_dumpInfos
getFaceEmotionVersion(instance)

# 测试传递结构体
class RecognizeResult(Structure):
    _fields_ = [
        ("code", ctypes.c_int),
        ("name", ctypes.c_char * 100),
        ("confidenceLevel", ctypes.c_float)
    ]

recognizeFunc = fe.fe_recognize
recognizeFunc.restype = (RecognizeResult)
ret = recognizeFunc(instance, bytes('/dummy',encoding='utf-8'))
print("code:", ret.code)
print("name:", ret.name)
print("confidenceLevel:", ret.confidenceLevel)

