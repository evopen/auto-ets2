from models.scnn_pytorch.model import SCNN
import torch
import os
import cv2
import numpy as np
import time
import torchvision

MODEL_DIR = '../models/scnn_pytorch'
INPUT_SIZE = (800, 288)

net = SCNN(input_size=INPUT_SIZE, pretrained=False, test=True)
state = torch.load(MODEL_DIR + '/exp10_best.pth')
net.load_state_dict(state['net'])
net.eval().cuda().half()

mean = (0.3598, 0.3653, 0.3662)
std = (0.2573, 0.2663, 0.2756)
drive_window_region = np.array([600.0 / 1920, 280.0 / 1080, 1000.0 / 1920, 360.0 / 1080])


def GetRectFromRegion(region: np.array, width, height):
    x = float(width) * region[0]
    y = float(height) * region[1]
    region_width = float(width) * region[2]
    region_height = float(height) * region[3]
    return np.array([x, y, region_width, region_height], dtype=np.int)


rect = GetRectFromRegion(drive_window_region, 1920, 1080)

for filename in os.listdir('../images'):
    print(filename)
    img = cv2.imread('../images/' + filename)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    drive_window_img = img[rect[1]:rect[1] + rect[3], rect[0]:rect[0] + rect[2]]
    input_blob = drive_window_img.astype(np.float)
    input_blob = input_blob / 255.0
    input_blob = (input_blob - mean) / std
    input_blob = cv2.resize(input_blob, (800, 288))
    input_blob = input_blob.transpose([2, 0, 1])

    input_tensor = torch.from_numpy(np.array([input_blob]))
    input_tensor = input_tensor.float().cuda().half()

    seg, exist = net(input_tensor)
    seg_img = seg.cpu().numpy().astype(np.uint8)

    seg_img = cv2.cvtColor(seg_img, cv2.COLOR_GRAY2RGB)
    seg_img = cv2.resize(seg_img, (1280, 720), interpolation=cv2.INTER_LINEAR)
    drive_window_img = cv2.resize(drive_window_img, (1280, 720))
    blend = cv2.addWeighted(seg_img * 50, 0.5, drive_window_img, 0.8, 0)

    cv2.imshow('asdf', blend)
    if cv2.waitKey(1) == 27:
        break
