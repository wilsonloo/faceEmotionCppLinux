import sys
sys.path.append('/usr/local/lib64')
sys.path.append('/home/wilson/faceEmotionCppLinux/lib')

import ctypes
from ctypes import *

# 加载内部库
fe = cdll.LoadLibrary("libfaceEmotion.so")   

# 获取版本信息
getFaceEmotionVersion = fe.fe_getVersion
getFaceEmotionVersion.restype = ctypes.c_char_p
print(getFaceEmotionVersion())

# 测试传递结构体
class RecognizeResult(Structure):
    _fields_ = [
        ("code", ctypes.c_int),
        ("name", ctypes.c_char * 100),
        ("confidenceLevel", ctypes.c_float)
    ]

recognizeFunc = fe.fe_recognize
recognizeFunc.restype = (RecognizeResult)
ret = recognizeFunc(bytes('/dummy',encoding='utf-8'))
print(ret)
print("code:", ret.code)
print("name:", ret.name)
print("confidenceLevel:", ret.confidenceLevel)

