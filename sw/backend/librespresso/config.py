"""Handles loading, saving, and definition of the configuration"""

import logging

import pydantic

logger = logging.getLogger(__name__)


class PIDParams(pydantic.BaseModel):
    """PID parameters."""
    p: float = 0.0
    i: float = 0.0
    d: float = 0.0


class LinearCalParams(pydantic.BaseModel):
    """Linear calibration parameters."""
    m: float = 1.0
    b: float = 0.0


class Config(pydantic.BaseModel):
    """Configuration for the librespresso backend."""
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
    """Load a configuration from a file."""
    try:
        return Config.parse_file(path)
    except FileNotFoundError as e:
        logger.info(
            'Error loading config file: %s, creating default config file at %s', e, path)
        c = Config()
        with open(path, 'w', encoding='utf-8') as f:
            f.write(c.json())
        return c
