import cv2
import mediapipe as mp
import pyrealsense2 as rs
import numpy as np

mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

_PRESENCE_THRESHOLD = 0.5
_VISIBILITY_THRESHOLD = 0.5

pipe = rs.pipeline()


#Set to false to use webcam
useDepth = True

#Use webcam if can't connect to RealSense
if (useDepth):
    try:
        profile = pipe.start()
    except:
        useDepth = False
        print("using webcam")
        cap = cv2.VideoCapture(0)
else:
    print("using webcam")
    cap = cv2.VideoCapture(0)

with mp_hands.Hands(
    model_complexity=0,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5) as hands:
  while (1):
    
    if (useDepth):
        frames = pipe.wait_for_frames()
        colorFrame = frames.get_color_frame()
        colorImage = np.asanyarray(colorFrame.get_data())
        colorImage = colorImage[:,:,::-1]

        depth = frames.get_depth_frame()
        depthImage = np.asanyarray(depth.get_data())

        image = colorImage
    else:
        success, image = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            # If loading a video, use 'break' instead of 'continue'.
            continue


    # To improve performance, optionally mark the image as not writeable to
    # pass by reference.
    image.flags.writeable = False
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    results = hands.process(image)

    # Draw the hand annotations on the image.
    image.flags.writeable = True
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
    if results.multi_hand_landmarks:
      count = 0
      for hand_landmarks in results.multi_hand_landmarks:
        mp_drawing.draw_landmarks(
            image,
            hand_landmarks,
            mp_hands.HAND_CONNECTIONS,
            mp_drawing_styles.get_default_hand_landmarks_style(),
            mp_drawing_styles.get_default_hand_connections_style())
        
        landmark_list = hand_landmarks
        image_rows, image_cols, _ = image.shape
        idx_to_coordinates = {}
        for idx, landmark in enumerate(landmark_list.landmark):
            if ((landmark.HasField('visibility') and
                landmark.visibility < _VISIBILITY_THRESHOLD) or
                (landmark.HasField('presence') and
                landmark.presence < _PRESENCE_THRESHOLD)):
                continue
            landmark_px = ((int)(landmark.x * image_cols),(int)(landmark.y * image_rows))
            if landmark_px:
                idx_to_coordinates[idx] = landmark_px
        ave = 0
        pixelCount = 0
        for pixel in idx_to_coordinates:
            pixelCount += 1
            if idx_to_coordinates[pixel][0] < image_cols and idx_to_coordinates[pixel][1] < image_rows:
                if (useDepth):
                    ave = ave + (depthImage[idx_to_coordinates[pixel][0],idx_to_coordinates[pixel][1]]/256)
                cv2.circle(image, idx_to_coordinates[pixel], 12, mp_drawing.WHITE_COLOR,6)
        ave /= pixelCount
        count += 1
        print(count,ave)

    # Flip the image horizontally for a selfie-view display.
    cv2.imshow('MediaPipe Hands', cv2.flip(image, 1))
    if cv2.waitKey(5) & 0xFF == 27:
      break