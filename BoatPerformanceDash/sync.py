import os
import requests
import glob
import shutil
from dotenv import load_dotenv

load_dotenv()

INFLUX_URL = os.getenv("INFLUX_URL")
INFLUX_TOKEN = os.getenv("INFLUX_TOKEN")
INFLUX_ORG = os.getenv("INFLUX_ORG")
INFLUX_BUCKET = os.getenv("INFLUX_BUCKET")
CF_CLIENT_ID = os.getenv("CF_SERVICE_TOKEN_CLIENT_ID")
CF_CLIENT_SECRET = os.getenv("CF_SERVICE_TOKEN_CLIENT_SECRET")

DASH_DIR = "/home/hydroliftpi/BoatPerformanceLogger/BoatPerformanceDash"
# Bind folders and make sure they exists.

PENDING_DIR = os.path.join(DASH_DIR, "pending")
SYNCED_DIR = os.path.join(DASH_DIR, "synced")

def send_file(path):
    with open(path) as f:
        data = f.read()
    # Create variables to send data
    url = f"{INFLUX_URL}/api/v2/write"
    params = {"org": INFLUX_ORG, "bucket": INFLUX_BUCKET, "precision": "ns"}
    headers = {
        "Authorization": f"Token {INFLUX_TOKEN}",
        "CF-Access-Client-Id": CF_CLIENT_ID,
        "CF-Access-Client-Secret": CF_CLIENT_SECRET,
    }
    # Send data to Influx
    r = requests.post(url, params=params, headers=headers, data=data, timeout=10)
    return r.status_code

def sync_all():
    # Read all files from pending folder
    files = sorted(glob.glob(os.path.join(PENDING_DIR, "*.influx")))
    if not files:
        return

    for path in files:
        try:
            # Send files
            code = send_file(path)
            # Check if file sent succesfully
            if code == 204:
                name = os.path.basename(path)
                shutil.move(path, os.path.join(SYNCED_DIR, name))
                print(f"synced: {name}")
            else:
                print(f"Failed ({code}): {path} - lämnar i pending")
                break
        except requests.RequestException as e:
            print(f"Ingen anslutning: {e} - försöker nästa gång")
            break

if __name__ == "__main__":
    sync_all()


