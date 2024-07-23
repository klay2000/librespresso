"""Handles communication with the microcontroller over UART."""

import logging
import threading
import time

import serial

from librespresso.command import Command

logger = logging.getLogger(__name__)

# TODO: deal with this global variable
_command_callbacks = {}
HB_SEND_INTERVAL = 1

tx_semaphore = threading.Semaphore()


class Uart:
    """Handles communication with the microcontroller over UART."""

    def __init__(self, port: str, baudrate: int):
        """Initialize the UART interface."""
        # logger.debug(f'Initializing UART on port {port} with baudrate {baudrate}')
        self._serial = serial.Serial(port, baudrate)
        # logger.debug(f'UART initialized on port {port} with baudrate {baudrate}')
        self._hb_thread = threading.Thread(target=self._hb_thread_fn)
        self._rx_thread = threading.Thread(target=self._rx_thread_fn)
        self._rx_thread.start()

    def send(self, command: Command):
        """Send a command to the microcontroller."""
        tx_semaphore.acquire()
        # logger.debug(f'Sending command: {command}')
        self._serial.write(command.to_bytes())
        tx_semaphore.release()

    def uart_callback(self, command: Command):
        """Decorator for registering a callback for a command."""
        def decorator(callback):
            _command_callbacks[command] = callback
            return callback
        return decorator

    def start_heartbeat(self):
        """Start sending heartbeat messages."""
        self._hb_thread.start()

    def _hb_thread_fn(self):
        """Heartbeat thread that sends a heartbeat message every second."""
        while True:
            # logger.debug('Sending heartbeat...')
            self.send(Command.heartbeat())
            time.sleep(1)

    def _rx_thread_fn(self):
        """Read thread that reads from the serial port and parses commands."""
        command_buffer = []
        while True:
            command_buffer.append(int.from_bytes(self._serial.read(),
                                                 byteorder='big',
                                                 signed=False))
            # trim buffer if too long
            if len(command_buffer) > 255+5:
                for i, _ in enumerate(command_buffer):
                    if command_buffer[i] == 0x02:
                        command_buffer = command_buffer[i:]
                        break
                # if command buffer is still too long then discard it
                if len(command_buffer) > 255+5:
                    command_buffer = []

            # if end of command was recieved then try to parse every possible command
            # check length to avoid out of bounds error from clear
            if len(command_buffer) > 0 and command_buffer[-1] == 0x03:
                for i, _ in enumerate(command_buffer):
                    if command_buffer[i] == 0x02:
                        try:
                            command = Command.from_bytes(command_buffer[i:])
                            # logger.debug(f'Command received: {command}')
                            # call callback if it exists and clear buffer
                            if command.command_type in _command_callbacks:
                                _command_callbacks[command.command_type](
                                    command)
                            command_buffer = []
                            break
                        except ValueError as e:
                            logger.error('Invalid pkt received: %s', e)
