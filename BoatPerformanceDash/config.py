import json
import os
import time
from dotenv import load_dotenv

load_dotenv()

CONFIG_PATH = "/home/hydroliftpi/BoatPerformanceLogger/BoatPerformanceSettings/config.json"
ESP_PIN = os.getenv("ESP_PIN", "")
ESP_CONFIG_PATH = "/home/hydroliftpi/BoatPerformanceLogger/BoatPerformanceSettings/esp_config.json"

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
    "slip_warn": 12,
    "slip_bad": 18
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
    "slip_warn": (0, 100),
    "slip_bad": (0, 100),
}

ESP_DEFAULTS =  {
        "ver": 0,
    "trim_ratio_a": 0.05, "trim_ratio_b": 0.45, "trim_val_a": 0, "trim_val_b": 100, "trim_alpha": 0.2,
    "tank_ratio_a": 0.05, "tank_ratio_b": 0.45, "tank_val_a": 0, "tank_val_b": 100, "tank_alpha": 0.01,
    "wp_zero_v": 0.5, "wp_fs": 30, "wp_alpha": 0.3,
    "pulses_per_rev": 12,
}

ESP_LIMITS = {
    "trim_ratio_a": (0, 1), "trim_ratio_b": (0, 1), "trim_val_a": (-100, 200), "trim_val_b": (-100, 200), "trim_alpha": (0.001, 1),
    "tank_ratio_a": (0, 1), "tank_ratio_b": (0, 1), "tank_val_a": (0, 200), "tank_val_b": (0, 200), "tank_alpha": (0.001, 1),
    "wp_zero_v": (0.3, 1.0), "wp_fs": (1, 300), "wp_alpha": (0.001, 1),
    "pulses_per_rev": (1, 100),
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
    if not check_parameters(config, LIMITS):
        return False
    tmp = CONFIG_PATH + ".tmp"
    with open(tmp, "w") as f:
        json.dump(config, f, indent=2)
        f.flush()
        os.fsync(f.fileno())
    os.replace(tmp, CONFIG_PATH)
    return True

def load_esp_config():
    try:
        with open(ESP_CONFIG_PATH) as f:
            data = json.load(f)            
        return {**ESP_DEFAULTS, **data}
    except (FileNotFoundError, json.JSONDecodeError):
        return ESP_DEFAULTS.copy()

def save_esp_config(config):    
    if not check_parameters(config, ESP_LIMITS):
        return None

    cfg = {key: config[key] for key in ESP_LIMITS}
    cfg["ver"] = int(time.time())

    tmp = ESP_CONFIG_PATH + ".tmp"
    with open(tmp, "w") as f:
        json.dump(cfg, f, indent=2)
        f.flush()
        os.fsync(f.fileno())
    os.replace(tmp, ESP_CONFIG_PATH)
    
    return cfg["ver"]


def check_parameters(param, limits=LIMITS):
    for key, (low, high) in limits.items():
        val = param.get(key)
        if not isinstance(val, (int, float)) or not (low <= val <= high):
            return False
    return True   
