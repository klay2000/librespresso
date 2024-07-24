"""Handles communication between the SOC and the MCU."""

from enum import IntEnum
import logging
import struct
from typing import Self

logger = logging.getLogger(__name__)


class CommandType(IntEnum):
    """Types of commands that can be sent to the MCU."""
    HB = 0
    CONFIG = 1
    DEBUG = 2
    RESET = 3
    START = 4


class ConfigParameter(IntEnum):
    """Configuration parameters for the MCU."""
    PUMP_MAX_RPM = 0
    PUMP_PID = 1
    PUMP_PRESSURE_CAL = 2


class Command:
    """Represents a command recieved from or to be sent to the MCU."""

    def __init__(self, command_type: CommandType, data: bytearray):
        self.command_type: CommandType = command_type
        self.data: bytearray = data

    def to_bytes(self) -> bytearray:
        """Convert the command to a byte array."""
        return bytearray([0x02, len(self.data), self.checksum(), self.command_type])\
            + self.data\
            + bytearray([0x03])

    def checksum(self) -> int:
        """Calculate the checksum for the command."""
        return (sum(self.data)+self.command_type) & 0xff

    @staticmethod
    def from_bytes(data: bytearray) -> Self:
        """Create a command from a byte array."""
        if data[0] != 0x02 or data[-1] != 0x03:
            raise ValueError("Invalid framing bytearray")
        if data[1] != len(data)-5:
            raise ValueError(
                f"Invalid length byte, expected: {len(data)-5}, received: {data[1]}")
        calculated_checksum = (sum(data[3:-1])) & 0xff
        if data[2] != calculated_checksum:
            raise ValueError(
                f"Invalid checksum, received: {data[2]}, calculated: {calculated_checksum}")
        return Command(CommandType(data[3]), data[4:-1])

    @staticmethod
    def heartbeat() -> Self:
        """Create a heartbeat command."""
        return Command(CommandType.HB, bytearray([]))

    @staticmethod
    def config(parameter: ConfigParameter, data: bytearray) -> Self:
        """Create a configuration command."""
        return Command(CommandType.CONFIG, bytearray(int(parameter).to_bytes(1, 'big'))+data)

    @staticmethod
    def reset() -> Self:
        """Create a reset command."""
        return Command(CommandType.RESET, bytearray([]))

    @staticmethod
    def start() -> Self:
        """Create a start command."""
        return Command(CommandType.START, bytearray([]))
