import socket
import threading
import json
import time

from datetime import datetime
from flask import Flask, jsonify, send_from_directory


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
    name = datetime.now().strftime("logs/trip_%Y-%m-%d_%H%M%S.influx")
    logfile = open(name, "a")
    buffer = []
    last_flush = time.time()

    while True:
        data, addr = sock.recvfrom(1024)
        # Load JSON Data
        try: 
            d = json.loads(data.decode())
            latest = d
            latest["pi_time"] = datetime.now().isoformat(timespec='milliseconds')

            # Logfile
            buffer.append(build_line(d))

            if time.time() - last_flush > 1.0:
                if buffer:
                    logfile.write("\n".join(buffer) + "\n") #Join all rows in buffer
                    logfile.flush()
                    buffer.clear()
                last_flush = time.time()
            #line = build_line(d)
            #logfile.write(line + "\n")
            #logfile.flush()

            ######### FIXME ###############
            # Until real sensors are connected
            latest["overheat"] = 0
            latest["oilLow"] = 0
        except (json.JSONDecodeError, ...):
            latest["link"] = 0
            continue



def build_line(d):
    ts = time.time_ns()
    return (f"boat "
            f"rpm={float(d.get('rpm',0))},"
            f"lift={float(d.get('lift',0))},"
            f"trim={float(d.get('trim',0))},"
            f"kn={float(d.get('kn',0))},"
            f"waterpressure={float(d.get('waterpressure',0))},"
            f"fuel={float(d.get('fuel',0))},"
            f"overheat={int(d.get('overheat',0))}i,"
            f"oilLow={int(d.get('oilLow',0))}i "
            f"{ts}")
# Configure Flask Webserver
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

# Start UDP And Flask on different Threads
if __name__ == "__main__":
    #Separate threads for UDP and Flask
    threading.Thread(target=udp_listener, daemon=True).start()
    app.run(host="0.0.0.0", port=5000)