import sys
sys.path.append('/usr/local/lib64')
sys.path.append('/home/wilson/faceEmotionCppLinux/lib')
#import librecognizePso;

import ctypes  
librecognizePso = ctypes.cdll.LoadLibrary("librecognizePso.so")   
getFaceEmotionVersion = librecognizePso.GetFaceEmotionVersion
getFaceEmotionVersion.restype = ctypes.c_char_p
print(getFaceEmotionVersion())
