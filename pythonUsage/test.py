import sys
sys.path.append('/usr/local/lib64')
sys.path.append('/home/wilson/faceEmotionCppLinux/lib')

import ctypes  
fe = ctypes.cdll.LoadLibrary("libfaceEmotion.so")   
getFaceEmotionVersion = fe.fe_getVersion
getFaceEmotionVersion.restype = ctypes.c_char_p
print(getFaceEmotionVersion())
