import socket
from pynput.mouse import Controller

mouse = Controller()

HOST = "0.0.0.0"
PORT = 5005

# Sensitivity (higher = more movement per tilt)
scale = 8  

# Smoothing factor (0.0 = no movement, 1.0 = instant jump)
alpha = 0.6  # higher means more responsive, lower means smoother

# Store smoothed values
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
            x_str, y_str = line.split(",")
            raw_dx = float(x_str) * scale
            raw_dy = float(y_str) * -scale  # invert Y axis

            # Low-pass filter for smoothing but keep it responsive
            smooth_dx = alpha * raw_dx + (1 - alpha) * smooth_dx
            smooth_dy = alpha * raw_dy + (1 - alpha) * smooth_dy

            print(f"Moving mouse by: {smooth_dx:.2f}, {smooth_dy:.2f}")
            mouse.move(smooth_dx, smooth_dy)
        except Exception as e:
            print("Error processing line:", line, e)
