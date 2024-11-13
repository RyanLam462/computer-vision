import cv2
from skimage.color import rgb2gray
import pyrealsense2 as rs
import matplotlib.pyplot as plt
from scipy import ndimage
import numpy as np
from sklearn.cluster import KMeans

cap = cv2.VideoCapture(0)
print("running")
while(1):
    success, image = cap.read()

    image = image/255

    if not success:
        print("Ignoring empty camera frame.")
        continue

    flatImage = image.reshape(image.shape[0]*image.shape[1], image.shape[2])

    kmeans = KMeans(n_clusters=8, random_state=0, max_iter = 10,tol=.005,algorithm="elkan").fit(flatImage)

    imShow = kmeans.cluster_centers_[kmeans.labels_]

    clusterImage = imShow.reshape(image.shape[0], image.shape[1], image.shape[2])

    cv2.imshow('Cube Detect', cv2.flip(clusterImage, 1))

    if cv2.waitKey(1) & 0xFF == 27:
        break

