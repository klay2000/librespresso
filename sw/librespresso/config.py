"""Handles loading, saving, and definition of the configuration"""

import json
import pydantic
import logging
import os

logger = logging.getLogger(__name__)

class PIDParams(pydantic.BaseModel):
  p: float = 0.0
  i: float = 0.0
  d: float = 0.0

class LinearCalParams(pydantic.BaseModel):
  m: float = 1.0
  b: float = 0.0

class Config(pydantic.BaseModel):
  # General parameters
  mcu_serial_port: str = "/dev/ttyUSB0"
  mcu_baudrate: int = 9600
  heartbeat_interval: float = 1.0

  # Pump parameters
  pump_pid: PIDParams = PIDParams()
  pump_rpm_max: float = 0.0

  # Sensor parameters
  pump_pressure_calibration: LinearCalParams = LinearCalParams()

def load_config(path: str) -> Config:
  try:
    return Config.parse_file(path)
  except Exception as e:
    logger.info(f"Error loading config file: {e}, creating default config file at {path}")
    c = Config()
    with open(path, 'w') as f:
      f.write(c.json())
    return c