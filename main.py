import socket
from pynput.mouse import Controller, Button

mouse = Controller()

HOST = "0.0.0.0"
PORT = 5005

scale = 5     # sensitivity
alpha = 1   # smoothing factor
noise_threshold = 0.1 * scale  # deadzone based on sensitivity

smooth_dx = 0
smooth_dy = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((HOST, PORT))
sock.listen(1)
print("Listening for ESP8266...")

conn, addr = sock.accept()
print(f"Connected by {addr}")

buffer = ""

while True:
    chunk = conn.recv(1024).decode()
    if not chunk:
        break
    buffer += chunk
    while "\n" in buffer:
        line, buffer = buffer.split("\n", 1)
        line = line.strip()
        if not line:
            continue
        try:
            ax_str, ay_str, left_str, right_str = line.split(",")
            raw_dx = float(ax_str) * scale
            raw_dy = -float(ay_str) * scale


            # ---- Deadzone to avoid jitter ----
            if abs(raw_dx) < noise_threshold:
                raw_dx = 0
            if abs(raw_dy) < noise_threshold:
                raw_dy = 0

            # ---- Smoothing ----
            smooth_dx = alpha * raw_dx + (1 - alpha) * smooth_dx
            smooth_dy = alpha * raw_dy + (1 - alpha) * smooth_dy

            # ---- Move mouse ----
            mouse.move(smooth_dx, smooth_dy)

            # ---- Click handling ----
            if left_str == "1":
                mouse.press(Button.left)


            if right_str == "1":
                mouse.press(Button.right)


        except Exception as e:
            print("Error processing line:", line, e)
