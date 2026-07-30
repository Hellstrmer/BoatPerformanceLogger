import socket
import threading
from datetime import datetime
from flask import Flask, jsonify, send_from_directory
import json

# Delad data
latest = {}

DASH_DIR = "/home/hydroliftpi/BoatPerformanceLogger/BoatPerformanceDash"

def udp_listener():
    UDP_IP = "0.0.0.0"
    UDP_PORT = 5005

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    print(f"Lyssnar på UDP {UDP_PORT}...")

    global latest

    while True:
        data, addr = sock.recvfrom(1024)

        try: 
            latest = json.loads(data.decode())
            latest["pi_time"] = datetime.now().isoformat(timespec='milliseconds')
        except (json.JSONDecodeError, ...):
            continue
        #print(f"{timestamp}  rpm: {latest["rpm"]}  lift: {latest["lift"]}")

app = Flask(__name__)

@app.route("/status")
def status():
    return jsonify(latest)

@app.route("/")
def index():
    return send_from_directory(DASH_DIR, "index.html")

@app.route("/<path:filename>")
def files(filename):
    return send_from_directory(DASH_DIR, filename)


if __name__ == "__main__":
    #Separate threads for UDP and Flask
    threading.Thread(target=udp_listener, daemon=True).start()
    app.run(host="0.0.0.0", port=5000)