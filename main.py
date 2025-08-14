import socket
from pynput.mouse import Controller, Button

mouse = Controller()

HOST = "0.0.0.0"
PORT = 5005

scale = 100   # sensitivity
alpha = 1   # smoothing
import socket
from pynput.mouse import Controller, Button

mouse = Controller()

HOST = "0.0.0.0"
PORT = 5005

scale = 50   # sensitivity
alpha = 1   # smoothing

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
            y_str, x_str, left_str, right_str = line.split(",")
            raw_dx = float(x_str) * scale
            raw_dy = float(y_str) * -scale

            smooth_dx = alpha * raw_dx + (1 - alpha) * smooth_dx
            smooth_dy = alpha * raw_dy + (1 - alpha) * smooth_dy


            # Handle left click
            if left_str == "1":
                mouse.press(Button.left)
            else:
                mouse.release(Button.left)

            # Handle right click
            if right_str == "1":
                mouse.press(Button.right)
            else:
                mouse.release(Button.right)

        except Exception as e:
            print("Error processing line:", line, e)

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
            x_str, y_str, left_str, right_str = line.split(",")
            raw_dx = float(x_str) * scale
            raw_dy = float(y_str) * -scale

            smooth_dx = alpha * raw_dx + (1 - alpha) * smooth_dx
            smooth_dy = alpha * raw_dy + (1 - alpha) * smooth_dy

            mouse.move(smooth_dx, smooth_dy)

            # Handle left click
            if left_str == "1":
                mouse.press(Button.left)
            else:
                mouse.release(Button.left)

            # Handle right click
            if right_str == "1":
                mouse.press(Button.right)
            else:
                mouse.release(Button.right)

        except Exception as e:
            print("Error processing line:", line, e)
