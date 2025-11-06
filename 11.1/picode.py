# --- Smart Glasses Main Program ---
# YOLOX11n + pyttsx3 voice + serial data from Arduino

import cv2
import time
import serial
import pyttsx3
from ultralytics import YOLO

# ==========================================
# CONFIGURATION
# ==========================================
MODEL_PATH = "yolox11n.pt"      # change if different
SERIAL_PORT = "/dev/ttyUSB0"    # adjust if /dev/ttyACM0
BAUD_RATE = 9600
VOICE_RATE = 175
SPEECH_INTERVAL = 1.5           # seconds between announcements
CONF_THRESH = 0.5

print("[INFO] Initializing system...")

# ==========================================
# SERIAL CONNECTION
# ==========================================
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    arduino_connected = True
    print("Arduino connected via USB.")
except:
    ser = None
    arduino_connected = False
    print("Arduino not detected. Running camera + voice only.")

# ==========================================
# YOLO + VOICE SETUP
# ==========================================
model = YOLO(MODEL_PATH)
labels = model.names

engine = pyttsx3.init()
engine.setProperty('rate', VOICE_RATE)
engine.setProperty('volume', 1.0)
last_spoken = 0
last_message = ""

def speak(text):
    global last_spoken, last_message
    now = time.time()
    if now - last_spoken > SPEECH_INTERVAL and text != last_message:
        engine.say(text)
        engine.runAndWait()
        last_spoken = now
        last_message = text

# ==========================================
# CAMERA SETUP
# ==========================================
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("ERROR: Cannot access camera.")
    exit()

print("[INFO] Starting real-time detection... Press 'q' to stop.")

# ==========================================
# MAIN LOOP
# ==========================================
while True:
    ret, frame = cap.read()
    if not ret:
        print("[WARN] No frame received.")
        break

    results = model(frame, verbose=False)
    detections = results[0].boxes
    h, w, _ = frame.shape
    positions = []

    for det in detections:
        conf = det.conf.item()
        if conf < CONF_THRESH:
            continue
        clsid = int(det.cls.item())
        label = labels[clsid]
        xyxy = det.xyxy.cpu().numpy().squeeze()
        xmin, ymin, xmax, ymax = xyxy.astype(int)
        center_x = (xmin + xmax) // 2

        # Determine left/center/right
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

    # Distance data from Arduino
    if arduino_connected and ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        if line.isdigit():
            distance = int(line)
            print(f"Distance: {distance} cm")
            if distance < 30:
                speak("Warning, obstacle very close")

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# ==========================================
# CLEANUP
# ==========================================
cap.release()
cv2.destroyAllWindows()
if arduino_connected:
    ser.close()
print("[INFO] System shut down cleanly.")
