import tensorflow as tf
import numpy
import os
import io
from io import BytesIO
import numpy as np
import cv2
from PIL import Image
from tensorflow import keras
from flask import Flask
from flask import request

config = tf.compat.v1.ConfigProto(allow_soft_placement=True)
gpu_options = tf.compat.v1.GPUOptions(per_process_gpu_memory_fraction=0.7)
config.gpu_options.allow_growth = True
with tf.compat.v1.Session(config=config) as sess:
    print("gpu set")    

app = Flask(__name__)

# 分类
label_names = ["anger", "disgust", "fear", "happy", "sad"]
label_to_index = dict((name, index) for index, name in enumerate(label_names))

# 加载模型
model = keras.models.load_model('emotion.model.mobilenetV2_x75_1.00_20200913')

# 从概率列表里，获取分类
def get_category_from_possiable(possiables):
    index = 0
    maxPoss = 0.0
    for k in range(len(possiables)):
        if possiables[k] > maxPoss:
            index = k
            maxPoss = possiables[k]
    return index, maxPoss

# 预处理图片
def preprocess_image(image, toDecode):
    fixedImg = image
    if toDecode == True:
        fixedImg = tf.image.decode_jpeg(image, channels=3)
    fixedImg = tf.image.convert_image_dtype(fixedImg,dtype=tf.float32)
    fixedImg = (fixedImg - 0.0)/255.0 # 归一化mean=0， std=255
    return fixedImg


# 进行分类
# @param iamgeData 尺寸为192*192 的图片
def classify_image(model, imageData, toDecode):
    fixedImage = preprocess_image(imageData, toDecode)
    inputBatch = fixedImage[np.newaxis, ...]
    predict = model.predict(inputBatch)
    possiables = predict[0]
    category, confidence = get_category_from_possiable(possiables)
    return category, confidence

@app.route('/')
def index():
    return "hello world"

@app.route('/classify_emotion', methods=['POST'])
def req_classify_emotion():
    if request.method == 'POST':
        binaryFlag = request.args.get('binary')
        toDecode = False
        imageBinary = request.get_data() 
        print("###### 1", type(imageBinary))
        
        image = cv2.imdecode(np.frombuffer(imageBinary, dtype=np.uint8), cv2.IMREAD_COLOR)
        print("$$$$$$ 2", type(image))

        category, confidence = classify_image(model, image, toDecode)
        label = label_names[category]
        print(label, confidence)
        return label+":"+str(confidence)


if __name__ == "__main__":
    # 加载测试图片
    imagePath = '/home/wilson/workstation/TFResFaceEmotion/emotions/happy/happy_11.jpg'
    imageData = tf.io.read_file(imagePath)

    # 进行测试
    #category, confidence = classify_image(model, imageData)
    #label = label_names[category]
    #print(label, confidence)
    app.run(host="192.168.1.19", port=5000, debug=True)
