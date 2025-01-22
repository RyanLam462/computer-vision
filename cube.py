import numpy as np
import cv2



distance = -6
focalLength = -100*distance

cube = np.array(
    [
        [-1, -1,  distance-1],
        [ 1, -1,  distance-1],
        [ 1,  1,  distance-1],
        [-1,  1,  distance-1],
        [-1, -1, distance],
        [ 1, -1, distance],
        [ 1,  1, distance],
        [-1,  1, distance]
    ]
)

orthogProject = np.array(
    [
        [1,0,0],
        [0,1,0],
        [0,0,0]
    ]
)

perspectiveProject = np.array(
    [
        [1,0,0],
        [0,1,0],
        [0,0,1/focalLength]
    ]
)

def drawCube(matrix,img,center):
    newCube = np.copy(cube)

    for i in range(0,len(cube)):    
        position = np.matmul(cube[i],matrix)
        newCube[i] = [position[0]/position[2],position[1]/position[2],0]

    width = (len(img[0]))
    height = (len(img))

    scale = 1

    for point in newCube:
        cv2.circle(img,(int(point[0]* scale) +int(center[0]),int(point[1]* scale)+int(center[1])),5,(0,255,0))
    return

#drawCube(orthogProject)