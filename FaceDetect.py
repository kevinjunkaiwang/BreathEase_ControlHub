import cv2
import mediapipe as mp
import pyautogui
from cvzone.SerialModule import SerialObject
from cvzone.FaceDetectionModule import FaceDetector

# Disable PyAutoGUI fail-safe
pyautogui.FAILSAFE = False

# Initialize SerialObject for Arduino communication
arduino = SerialObject("COM4")

# Open the webcam
cam = cv2.VideoCapture(0)

# Create a FaceMesh object from Mediapipe library
face_mesh = mp.solutions.face_mesh.FaceMesh(refine_landmarks=True)

# Get Screen Size
screen_w, screen_h = pyautogui.size()

msg = ""

# Detect face
detector = FaceDetector(minDetectionCon=0.7)
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

# Initialize variables
eyes_closed = False
eyes_closed_start_time = 0
action_count = 0

# Adjust the sensitivity ratio
sensitivity_ratio = 2.0  # You can adjust this value

while True:
    success, img = cam.read()
    img = cv2.flip(img, 1)
    rgb_frame = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    output = face_mesh.process(rgb_frame)

    # Get landmark points if available
    landmark_points = output.multi_face_landmarks

    # Get frame dimensions
    frame_h, frame_w, _ = img.shape

    if landmark_points:
        landmarks = landmark_points[0].landmark  # Only one person

        for id, landmark in enumerate(landmarks[474:478]):
            x = int(landmark.x * frame_w)
            y = int(landmark.y * frame_h)
            cv2.circle(img, (x, y), 3, (0, 255, 0))
            if id == 1:
                # Adjust the sensitivity of cursor movement
                screen_x = screen_w * landmark.x
                screen_y = screen_h * landmark.y
                pyautogui.moveTo(screen_x, screen_y)

        # Extract additional landmarks for the left eye
        left = [landmarks[145], landmarks[159]]

        # Draw circles around the left eye landmarks
        for landmark in left:
            x = int(landmark.x * frame_w)
            y = int(landmark.y * frame_h)
            cv2.circle(img, (x, y), 3, (0, 255, 255))

        # Check if the left eye is closed (based on vertical distance between specific landmarks)
        if (left[0].y - left[1].y) < 0.004:
            pyautogui.click()
            pyautogui.sleep(0.1)

    # Detect whether the person is there using Haarcascades
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.3, minNeighbors=5)

    for (x, y, w, h) in faces:
        cv2.rectangle(img, (x, y), (x + w, y + h), (255, 0, 0), 2)

        # Send ASCII values separately
        arduino.sendData([ord("Y")])
        msg = "Patient Detected"

    if len(faces) == 0:
        arduino.sendData([ord("N")])
        msg = "No Patient Detected"

    print(msg)

    cv2.imshow('Face Detection', img)
    cv2.waitKey(1)
