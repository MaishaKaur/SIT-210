import os
import sys
import argparse
import glob
import time
import cv2
import numpy as np
import pyttsx3
from ultralytics import YOLO

# =============================
# Argument parsing (same as original)
# =============================
parser = argparse.ArgumentParser()
parser.add_argument('--model', required=True)
parser.add_argument('--source', required=True)
parser.add_argument('--thresh', default=0.5)
parser.add_argument('--resolution', default=None)
parser.add_argument('--record', action='store_true')
args = parser.parse_args()

# =============================
# Setup
# =============================
model_path = args.model
img_source = args.source
min_thresh = float(args.thresh)
user_res = args.resolution
record = args.record

if not os.path.exists(model_path):
    print('ERROR: Model path is invalid.')
    sys.exit(0)

model = YOLO(model_path, task='detect')
labels = model.names

# =============================
# Source setup (same as original)
# =============================
img_ext_list = ['.jpg','.JPG','.jpeg','.JPEG','.png','.PNG','.bmp','.BMP']
vid_ext_list = ['.avi','.mov','.mp4','.mkv','.wmv']

if os.path.isdir(img_source):
    source_type = 'folder'
elif os.path.isfile(img_source):
    _, ext = os.path.splitext(img_source)
    if ext in img_ext_list:
        source_type = 'image'
    elif ext in vid_ext_list:
        source_type = 'video'
    else:
        print(f'File extension {ext} not supported.')
        sys.exit(0)
elif 'usb' in img_source:
    source_type = 'usb'
    usb_idx = int(img_source[3:])
elif 'picamera' in img_source:
    source_type = 'picamera'
    picam_idx = int(img_source[8:])
else:
    print(f'Invalid source: {img_source}')
    sys.exit(0)

# =============================
# Resolution setup
# =============================
resize = False
if user_res:
    resize = True
    resW, resH = int(user_res.split('x')[0]), int(user_res.split('x')[1])

# =============================
# Voice setup
# =============================
engine = pyttsx3.init()
engine.setProperty('rate', 175)
engine.setProperty('volume', 1.0)
last_spoken = time.time()
last_message = ""
SPEECH_INTERVAL = 1.5

def speak(text):
    global last_spoken, last_message
    now = time.time()
    if now - last_spoken > SPEECH_INTERVAL and text != last_message:
        engine.say(text)
        engine.runAndWait()
        last_spoken = now
        last_message = text

# =============================
# Input source handling
# =============================
if source_type == 'image':
    imgs_list = [img_source]
elif source_type == 'folder':
    imgs_list = [f for f in glob.glob(img_source + '/*') if os.path.splitext(f)[1] in img_ext_list]
elif source_type in ['video', 'usb']:
    cap = cv2.VideoCapture(img_source if source_type == 'video' else int(img_source[3:]))
    if user_res:
        cap.set(3, resW)
        cap.set(4, resH)
elif source_type == 'picamera':
    from picamera2 import Picamera2
    cap = Picamera2()
    cap.configure(cap.create_video_configuration(main={"format": 'XRGB8888', "size": (resW, resH)}))
    cap.start()

# =============================
# Detection Loop
# =============================
print("[INFO] Starting real-time detection with voice output...")

while True:
    if source_type in ['video', 'usb', 'picamera']:
        ret, frame = cap.read() if source_type != 'picamera' else (True, cv2.cvtColor(cap.capture_array(), cv2.COLOR_BGRA2BGR))
        if not ret or frame is None:
            print("[WARN] No frame received, exiting.")
            break
    else:
        if len(imgs_list) == 0:
            print("No images found.")
            break
        frame = cv2.imread(imgs_list.pop(0))

    if resize:
        frame = cv2.resize(frame, (resW, resH))

    results = model(frame, verbose=False)
    detections = results[0].boxes
    h, w, _ = frame.shape
    positions = []

    for det in detections:
        conf = det.conf.item()
        if conf < min_thresh:
            continue
        clsid = int(det.cls.item())
        label = labels[clsid]
        xyxy = det.xyxy.cpu().numpy().squeeze()
        xmin, ymin, xmax, ymax = xyxy.astype(int)
        center_x = (xmin + xmax) // 2

        if center_x < w / 3:
            pos = "left"
        elif center_x > 2 * w / 3:
            pos = "right"
        else:
            pos = "center"

        positions.append(f"{label} on your {pos}")

    if positions:
        message = ", ".join(positions)
        print("[INFO]", message)
        speak(message)
    else:
        speak("clear path ahead")

    key = cv2.waitKey(1)
    if key == ord('q'):
        break

if source_type in ['video', 'usb']:
    cap.release()
cv2.destroyAllWindows()
