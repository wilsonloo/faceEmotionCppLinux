# faceEmotionCppLinux

## 第三方库要求
  1、opencv.3.4.6
    *考虑到对应so比较大，不再将对应include和lib拷贝或者独立创建git
    *安装方式：https://docs.opencv.org/3.4.6/d7/d9f/tutorial_linux_install.html

## 2020-09-03<br>
  总结：<br>
  1、完成了通过固定尺寸640*480 的png人像照片注册（通过配置目录）；<br>
  2、注册人脸信息到sqlite数据库，利于后续匹配<br>
  3、从配置目录加载需要识别的人像图片，检测和识别是谁。<br>
  <br>
  计划：<br>
  1、使用jypter 让python调用c++ 交互进行人脸检测与识别展示。<br>
  
