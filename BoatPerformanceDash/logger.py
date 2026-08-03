import socket
import threading
import json
import time
import os

from datetime import datetime
from flask import Flask, jsonify, send_from_directory, request
from config import load_config, save_config


# Delad data
latest = {}

DASH_DIR = "/home/hydroliftpi/BoatPerformanceLogger/BoatPerformanceDash"
SETTINGS_DIR = "/home/hydroliftpi/BoatPerformanceLogger/BoatPerformanceSettings"
# Bind folders and make sure they exists.
LOG_DIR = os.path.join(DASH_DIR, "logs")
os.makedirs(LOG_DIR, exist_ok=True)
PENDING_DIR = os.path.join(DASH_DIR, "pending")
os.makedirs(PENDING_DIR, exist_ok=True) 

def udp_listener():
    UDP_IP = "0.0.0.0"
    UDP_PORT = 5005

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    #Stop the socket from trying to recieve after 1 second
    sock.settimeout(1.0)
    print(f"Lyssnar på UDP {UDP_PORT}...")

    global latest
    logfile = None
    buffer = []
    last_flush = time.time()
    last_packet = time.time()

    SESSION_TIMEOUT = 30 # Session finished when no data arrives for this time

    while True:
        try: 
            data, addr = sock.recvfrom(1024)
            # Load JSON Data
            d = json.loads(data.decode())
            latest = d
            latest["pi_time"] = datetime.now().isoformat(timespec='milliseconds')
            last_packet = time.time()

            # Calculate the slip angle of prop
            config_values = load_config()
            latest["slip"] = calculate_slip(latest["rpm"], latest["kn"],config_values["prop_pitch"], config_values["prop_gear"])
            #Open new logfile if None is open
            if logfile is None:                
                name = datetime.now().strftime("trip_%Y-%m-%d_%H%M%S.influx")
                path = os.path.join(LOG_DIR, name)
                logfile = open(path, "a")
                print(f"New Session Started: {name}")

            # Logfile
            buffer.append(build_line(d))
        except socket.timeout:
            pass # Do nothing for the first time
        except json.JSONDecodeError:
            continue # Broken package, skip

        # Save data to log on interval
        if logfile and time.time() - last_flush > 1.0:
            if buffer:
                logfile.write("\n".join(buffer) + "\n") #Join all rows in buffer
                logfile.flush()
                buffer.clear()
            last_flush = time.time()

        ######### FIXME ###############
        # Until real sensors are connected
        latest["overheat"] = 0
        latest["oilLow"] = 0
        
        if logfile and time.time() - last_packet > SESSION_TIMEOUT:
            if buffer:
                logfile.write("\n".join(buffer) + "\n") #Join all rows in buffer
                logfile.flush()
                buffer.clear()
            logfile.close()
            logfile = None
            oldpath = os.path.join(LOG_DIR, name)
            newpath = os.path.join(PENDING_DIR, name)
            os.replace(oldpath, newpath)
            print("Session Finished")

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
            f"oilLow={int(d.get('oilLow',0))}i",
            f"oilLow={float(d.get('slip',0))}",
            f"{ts}")

def calculate_slip(rpm, kn, pitch, gear):
    if rpm < 400:
        return 0.0
    theoretical_kn = (rpm * pitch) / gear * 1215
    if theoretical_kn <= 0:
        return 0.0
    slip = (1 - kn/ theoretical_kn) * 100
    return max(0.0, slip)




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

# Settings
@app.route("/settings")
def settings_index():
    return send_from_directory(SETTINGS_DIR, "index.html")

@app.route("/settings/<path:filename>")
def settings_files(filename):
    return send_from_directory(SETTINGS_DIR, filename)

@app.route("/config")
def get_config():
    return jsonify(load_config())

@app.route("/config", methods=["POST"])
def set_config():
    new = request.get_json(force=True)
    if save_config(new):
        return jsonify({"status": "ok"})
    else: 
        return jsonify({"error": "ogiltiga parametrar"}), 400


# Start UDP And Flask on different Threads
if __name__ == "__main__":
    #Separate threads for UDP and Flask
    threading.Thread(target=udp_listener, daemon=True).start()
    app.run(host="0.0.0.0", port=5000, threaded=True)