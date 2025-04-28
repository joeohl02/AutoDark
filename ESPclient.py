"""
ESPclient.py
This Python script is used to manually control a curtain motor connected to an ESP32S3 device via HTTP requests.
Pressing the 'w' or 's' key will send an "up" or "down" command respectively.
Releasing the keys will send a "stop" command to halt motor movement.
"""

# Requires the 'requests' and 'keyboard' packages to be installed.
# Install them with: pip install requests keyboard

import requests
import time
import keyboard

# URLs for up, down, and stop commands.
# NOTE: Update the IP address below to match your ESP32 device's local IP address.
up_url = "http://192.168.1.175/up"
down_url = "http://192.168.1.175/down"
stop_url = "http://192.168.1.175/stop"


def on_w_press(event):
    try:
        response = requests.get(up_url, timeout=5)
        print("Up command sent. Response:", response.text)
    except Exception as e:
        print("Error sending up command:", e)


def on_s_press(event):
    try:
        response = requests.get(down_url, timeout=5)
        print("Down command sent. Response:", response.text)
    except Exception as e:
        print("Error sending down command:", e)


def on_key_release(event):
    try:
        response = requests.get(stop_url, timeout=5)
        print("Stop command sent after key release. Response:", response.text)
    except Exception as e:
        print("Error sending stop command:", e)


# Register callbacks for key press and release events.
keyboard.on_press_key("w", on_w_press)
keyboard.on_release_key("w", on_key_release)
keyboard.on_press_key("s", on_s_press)
keyboard.on_release_key("s", on_key_release)

print("Press 'w' to move up, 's' to move down. Release keys to stop movement. Press 'q' to exit.")

while True:
    if keyboard.is_pressed("q"):
        print("Exiting program.")
        break
    time.sleep(0.1)
