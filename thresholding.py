import numpy as np
import cv2
 

cap = cv2.VideoCapture(0)

minSat = 200
minVal = 200

while (1):
    success, img = cap.read()
    edges = cv2.Canny(img,200,200)

    saturationVal = 3

    ##SATURATE
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV).astype("float32")
    (h, s, v) = cv2.split(hsv)
    s = s*saturationVal
    s = np.clip(s,0,255)
    hsvSat = cv2.merge([h,s,v])
    saturated = cv2.cvtColor(hsvSat.astype("uint8"), cv2.COLOR_HSV2BGR)

    ##THRESHOLD
    greenMask = cv2.inRange(hsv, (36, 80, 80), (70, 255,255))
    gmask = greenMask>0
    #green = np.zeros_like(img, np.uint8)
    #green[gmask] = img[gmask]

    blueMask = cv2.inRange(hsv, (110, 0, 0), (130, 255,255))
    bmask = blueMask>0
    #blue = np.zeros_like(img, np.uint8)
    #blue[bmask] = img[bmask]

    redLMask = cv2.inRange(hsv, (0, 80, 80), (10, 255,255))
    rlmask = redLMask>0

    redHMask = cv2.inRange(hsv, (170, 80, 80), (180, 255,255))
    rhmask = redHMask>0



    satMask = cv2.inRange(hsv, (0, minSat, 0), (255, 255,255))
    satMask = satMask > 0

    vMask = cv2.inRange(hsv, (0, 0, minVal), (255, 255,255))
    vMask = vMask > 0

    threshold = np.zeros_like(img,np.uint8)

    
    #threshold[bmask] = img[bmask]

    #threshold[gmask] = img[gmask]

    #threshold[rlmask] = img[rlmask]

    #threshold[rhmask] = img[rhmask]

    threshold[satMask] = img[satMask]

    threshold[vMask] = img[vMask]


    cv2.imshow('canny',threshold)

    key = cv2.waitKey(5)

    

    if key != -1:
        if key == 27:        
            break
        elif key  == 119:
            minSat += 5
        elif key  == 97:
            minVal -= 5
        elif key  == 115:
            minSat -= 5
        elif key  == 100:
            minVal += 5
        elif key == 32:
            print(minSat,minVal)
        else:
            print(key)
    

