import os
from dotenv import load_dotenv

load_dotenv()

INFLUX_URL = os.getenv("INFLUX_URL")
INFLUX_TOKEN = os.getenv("INFLUX_TOKEN")
INFLUX_ORG = os.getenv("INFLUX_ORG")
INFLUX_BUCKET = os.getenv("IFNLUX_BUCKET")

CF_CLIENT_ID = os.getenv("CF_SERVICE_TOKEN_CLIENT_ID")
CF_CLIENT_SECRET = os.getenv("CF_SERVICE_TOKEN_CLIENT_SECRET")