import numpy as np
import cv2
import mediapipe as mp
import pyrealsense2 as rs
import cube

print("starting")
 
pipe = rs.pipeline()

useDepth = True

if useDepth:
    try:
        profile = pipe.start()
    except:
        useDepth = False
        print("using depth")
        cap = cv2.VideoCapture(0)
else:
    useDepth = False
    print("using webcam")
    cap = cv2.VideoCapture(0) 

minSat = 155

while (1):

    if (useDepth):
        frames = pipe.wait_for_frames()
        colorFrame = frames.get_color_frame()
        colorImage = np.asanyarray(colorFrame.get_data())
        colorImage = colorImage[:,:,::-1]
        img = colorImage
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        depth = frames.get_depth_frame()
        depthImage = np.asanyarray(depth.get_data())
        depthImage = cv2.cvtColor(depthImage, cv2.COLOR_BGR2RGB)
        depthImage = cv2.cvtColor(depthImage, cv2.COLOR_BGR2GRAY).astype("float32")

        difference = len(depthImage[0]) - len(img[0])

        depthImage = depthImage[0:len(depthImage),int(difference/2):len(depthImage[0])-int(difference/2)]


    else:
        success, img = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            # If loading a video, use 'break' instead of 'continue'.
            continue
    

    #edges = cv2.Canny(img,200,200)

    #img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB).astype("uint16")

    saturationVal = 3

    ##SATURATE
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV).astype("uint16")

    ##THRESHOLD
    satMask = cv2.inRange(hsv, (0, minSat, 0), (255, 255,255))
    satMask = satMask > 0

    threshold = np.zeros_like(img,np.uint16)

    threshold[satMask] = img[satMask]

    grayscale = cv2.cvtColor(threshold, cv2.COLOR_BGR2GRAY).astype("uint16")

    #if(useDepth):
        #diff = cv2.absdiff(grayscale,depthImage)

    #grayscale = cv2.GaussianBlur(grayscale, (25, 25), 20)
    (minv, maxv, minLoc, maxLoc) = cv2.minMaxLoc(grayscale)
    cv2.circle(img, maxLoc, 5, (255, 0, 0), 2)

    #cube.drawCube(cube.perspectiveProject,img,maxLoc)

    cv2.imshow('threshold',grayscale)

    key = cv2.waitKey(5)

    if key != -1:
        if key == 27:        
            break
    

