# --- Raspberry Pi Code for Smart Glasses ---
# Purpose:
#   1. Run YOLOv11 (Nano model) to detect objects in real-time using the Pi camera.
#   2. Use Arduino's ultrasonic distance readings if connected.
#   3. Give spoken feedback to the user via text-to-speech (pyttsx3).

import cv2
import time
import serial
import pyttsx3
from ultralytics import YOLO

# ==========================================
# CONFIGURATION SECTION
# ==========================================
MODEL_PATH = "yolox11n.pt"      # Pretrained YOLOv11 Nano model file
SERIAL_PORT = "/dev/ttyUSB0"    # USB port where Arduino is connected
BAUD_RATE = 9600                # Communication speed (must match Arduino)
VOICE_RATE = 175                # Speech rate for voice output
SPEECH_INTERVAL = 1.5           # Minimum time gap between spoken messages
CONF_THRESH = 0.5               # Minimum confidence score for YOLO detections

print("[INFO] Booting Smart Glasses System...")

# ==========================================
# CONNECT TO ARDUINO (OPTIONAL)
# ==========================================
try:
    # Try to open a serial connection to Arduino
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for Arduino to initialize
    arduino_connected = True
    print("✅ Arduino detected (distance integration active).")
except:
    # If Arduino is not connected, continue with only YOLO
    ser = None
    arduino_connected = False
    print("⚠️ No Arduino detected — running YOLO + voice only.")

# ==========================================
# LOAD YOLO MODEL AND SETUP VOICE ENGINE
# ==========================================
model = YOLO(MODEL_PATH)        # Load YOLOv11 Nano model for object detection
labels = model.names            # Retrieve object class labels (e.g., person, car)

# Initialize text-to-speech engine
engine = pyttsx3.init()
engine.setProperty('rate', VOICE_RATE)
engine.setProperty('volume', 1.0)
last_spoken = 0                 # Last time something was spoken
last_message = ""               # Store previous message to avoid repetition

def speak(text):
    """Speak only if enough time passed and message changed"""
    global last_spoken, last_message
    now = time.time()
    if now - last_spoken > SPEECH_INTERVAL and text != last_message:
        engine.say(text)
        engine.runAndWait()
        last_spoken = now
        last_message = text

# ==========================================
# INITIALIZE CAMERA
# ==========================================
cap = cv2.VideoCapture(0)       # Open default Pi camera
if not cap.isOpened():
    print("❌ ERROR: Cannot access camera.")
    exit()

print("[INFO] Detection running... Press 'q' to quit.")

# ==========================================
# MAIN LOOP
# ==========================================
while True:
    # --- Capture frame from camera ---
    ret, frame = cap.read()
    if not ret:
        continue  # Skip if frame not captured

    # --- Run YOLO detection on the frame ---
    results = model(frame, verbose=False)
    detections = results[0].boxes
    h, w, _ = frame.shape
    positions = []  # Store text messages for each detection

    # --- Loop through detected objects ---
    for det in detections:
        conf = det.conf.item()             # Confidence score
        if conf < CONF_THRESH:
            continue                       # Ignore low-confidence detections

        clsid = int(det.cls.item())        # Class ID
        label = labels[clsid]              # Class name (e.g. "person")
        xyxy = det.xyxy.cpu().numpy().squeeze()
        xmin, ymin, xmax, ymax = xyxy.astype(int)
        center_x = (xmin + xmax) // 2      # Center X coordinate of object

        # Determine which side of the frame object is on
        if center_x < w / 3:
            pos = "left"
        elif center_x > 2 * w / 3:
            pos = "right"
        else:
            pos = "center"

        positions.append(f"{label} on your {pos}")

    # --- Voice output for detections ---
    if positions:
        message = ", ".join(positions)
        speak(message)
    else:
        speak("clear path ahead")

    # ==========================================
    # READ DISTANCE DATA FROM ARDUINO
    # ==========================================
    if arduino_connected and ser.in_waiting > 0:
        # Read incoming data from Arduino (distance in cm)
        line = ser.readline().decode('utf-8').strip()
        if line.isdigit():
            distance = int(line)
            print(f"Distance: {distance} cm")

            # If an object is too close, warn the user
            if distance < 25:
                speak("Warning, object too close in front")

    # --- Exit when 'q' key is pressed ---
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# ==========================================
# CLEANUP
# ==========================================
cap.release()                    # Release camera
cv2.destroyAllWindows()          # Close all OpenCV windows

if arduino_connected:
    ser.close()                  # Close serial port if used

print("[INFO] System stopped cleanly.")
