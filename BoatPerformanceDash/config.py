import json
import os

CONFIG_PATH = "/home/hydroliftpi/BoatPerformanceLogger/BoatPerformanceSettings/config.json"

DEFAULTS = {
    "prop_gear": 2.00,
    "prop_pitch": 25,

    "redline": 5500,
    "rpm_max": 6200,
    "lift_min": 0,
    "lift_max": 154.2,
    "trim_min": 0,
    "trim_max": 45,
    "water_min": 10.0,
    "water_max": 1000.0,
    "fuel_min": 0,
    "fuel_max": 70,
}
LIMITS = {
    "prop_gear":  (0.0, 10.0),
    "prop_pitch": (0, 50),
    "redline":    (0, 10000),
    "rpm_max":    (0, 10000),
    "lift_min":   (0, 200),
    "lift_max":   (0, 200),
    "trim_min": (0.0, 200.0),
    "trim_max": (0.0, 200.0),
    "water_min": (0.0, 1000.0),
    "water_max": (0.0, 1000.0),
    "fuel_min": (0.0, 10000.0),
    "fuel_max": (0.0, 10000.0),
}


def load_config():
    try:
        with open(CONFIG_PATH) as f:
            data = json.load(f)            
        return {**DEFAULTS, **data}
    except (FileNotFoundError, json.JSONDecodeError):
        return DEFAULTS.copy()

def save_config(config):
    print(config)
    if not check_parameters(config):
        return False
    tmp = CONFIG_PATH + ".tmp"
    with open(tmp, "w") as f:
        json.dump(config, f, indent=2)
        f.flush()
        os.fsync(f.fileno())
    os.replace(tmp, CONFIG_PATH)
    return True

def check_parameters(param):
   for key, (low, high) in LIMITS.items():
    val = param.get(key)
    if val is None or not (low <= val <= high):
        return False
    return True
