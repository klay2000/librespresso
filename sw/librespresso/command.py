"""Handles communication between the SOC and the MCU."""
from enum import IntEnum
from typing import Self
import logging

logger = logging.getLogger(__name__)

class CommandType(IntEnum):
  HB = 0
  CONFIG = 1
  DEBUG = 2

class ConfigParameter(IntEnum):
  PUMP_MAX_RPM = 0
  PUMP_PID_P = 1
  PUMP_PID_I = 2
  PUMP_PID_D = 3
  PUMP_PRESSURE_CAL_M = 4
  PUMP_PRESSURE_CAL_B = 5

class Command:
  def __init__(self, command_type: CommandType, data: bytes):
    self.command_type = command_type
    self.data = data

  def to_bytes(self) -> bytes:
    return [0x02, len(self.data), self.checksum(), self.command_type] + self.data + [0x03]

  def checksum(self) -> int:
    return (sum(self.data)+self.command_type) & 0xff
  
  @staticmethod
  def from_bytes(data: bytes) -> Self:
    if data[0] != 0x02 or data[-1] != 0x03:
      raise ValueError("Invalid framing bytes")
    if data[1] != len(data)-5:
      raise ValueError(f"Invalid length byte, expected: {len(data)-5}, received: {data[1]}")
    calculated_checksum = (sum(data[3:-1])) & 0xff
    if data[2] !=  calculated_checksum:
      raise ValueError(f"Invalid checksum, received: {data[2]}, calculated: {calculated_checksum}")
    return Command(CommandType(data[3]), data[4:-1])
  
  @staticmethod
  def heartbeat() -> Self:
    return Command(CommandType.HB, [])
  
  @staticmethod
  def config(parameter: ConfigParameter, data: bytes) -> Self:
    return Command(CommandType.CONFIG, data)