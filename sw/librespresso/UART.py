"""Handles communication with the microcontroller over UART."""

import threading
import serial
import time
import logging
from command import Command

logger = logging.getLogger(__name__)

# TODO: deal with this global variable
_command_callbacks = {}
HB_SEND_INTERVAL = 1

tx_semaphore = threading.Semaphore()

class UART:
  def __init__(self, port: str, baudrate: int):
    # logger.debug(f'Initializing UART on port {port} with baudrate {baudrate}')
    self._serial = serial.Serial(port, baudrate)
    # logger.debug(f'UART initialized on port {port} with baudrate {baudrate}')
    self._hb_thread = threading.Thread(target=self._hb_thread)
    self._rx_thread = threading.Thread(target=self._read_thread)
    self._rx_thread.start()

  def send(self, command: Command):
    tx_semaphore.acquire()
    # logger.debug(f'Sending command: {command}')
    self._serial.write(command.to_bytes())
    tx_semaphore.release()
  
  def UART_callback(command: Command):
    def decorator(callback):
      _command_callbacks[command] = callback
      return callback
    return decorator

  def start_heartbeat(self):
    self._hb_thread.start()

  def _hb_thread(self):
    while True:
      # logger.debug('Sending heartbeat...')
      self.send(Command.heartbeat())
      time.sleep(1)

  def _read_thread(self):
    command_buffer = []
    while True:
      command_buffer.append(int.from_bytes(self._serial.read(), byteorder='big', signed=False))
      # trim buffer if too long
      if len(command_buffer) > 255+5:
        for i in range(len(command_buffer)):
          if command_buffer[i] == 0x02:
            command_buffer = command_buffer[i:]
            break
        # if command buffer is still too long then discard it
        if len(command_buffer) > 255+5:
          command_buffer = []
      
      # if end of command was recieved then try to parse every possible command
      # check length to avoid out of bounds error from clear
      if len(command_buffer) > 0 and command_buffer[-1] == 0x03:
        for i in range(len(command_buffer)):
          if command_buffer[i] == 0x02:
            try:
              command = Command.from_bytes(command_buffer[i:])
              # logger.debug(f'Command received: {command}')
              # call callback if it exists and clear buffer
              if command.command_type in _command_callbacks:
                _command_callbacks[command.command_type](command)
              command_buffer = []
              break
            except ValueError as e:
              logger.error(f'Invalid pkt received: {e}')
              pass
      
      # # wait for start byte
      # byte: int = 0x00
      # while byte != 0x02:
      #   # logger.debug("waiting for start byte...")
      #   byte = int.from_bytes(self._serial.read(), byteorder='big', signed=False)
      #   # logger.debug(f"byte ({byte}) received!")
      # logger.debug("start byte received!")
      
      # # initialize command buffer
      # command_buffer = [0x02]

      # # read command until end byte is received and parsable or buffer is too long
      # while len(command_buffer) < 255+5:
      #   # logger.debug("waiting for byte...")
      #   command_buffer.append(int.from_bytes(self._serial.read(), byteorder='big', signed=False))
      #   logger.debug(f'command_buffer: {command_buffer}')
      #   for i in range(len(command_buffer)):
      #     if command_buffer[i] == 0x02:
      #       logger.debug(f'start byte found at index {i}')
      #       if command_buffer[-1] == 0x03:
      #         try:
      #           command = Command.from_bytes(command_buffer[i:-1])
      #           logger.debug(f'Command received: {command}')
      #           # call callback if it exists
      #           if command.command_type in _command_callbacks:
      #             _command_callbacks[command.command_type](command)
      #           break
      #         except ValueError as e:
      #           logger.error(f'Invalid command received {e}')
      #           pass