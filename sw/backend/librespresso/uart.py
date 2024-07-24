"""Handles communication with the microcontroller over UART."""

import logging
import threading

import serial
import time

from librespresso.command import Command

logger = logging.getLogger(__name__)

tx_semaphore = threading.Semaphore()


class Uart:
    """Handles communication with the microcontroller over UART."""

    def __init__(self, port: str, baudrate: int):
        """Initialize the UART interface."""
        self._serial = serial.Serial(port, baudrate)
        self._rx_thread = threading.Thread(target=self._rx_thread_fn)
        self._rx_thread.start()
        self._command_callbacks = {}

    def send(self, command: Command):
        """Send a command to the microcontroller."""
        tx_semaphore.acquire()
        # logger.debug(f'Sending command: {command}')
        # logger.debug(f'Command bytes: {list(command.to_bytes())}')
        self._serial.write(command.to_bytes())
        tx_semaphore.release()

    def uart_callback(self, command: Command):
        """Decorator for registering a callback for a command."""
        def decorator(callback):
            self._command_callbacks[command] = callback
            return callback
        return decorator

    def _rx_thread_fn(self):
        """Read thread that reads from the serial port and parses commands."""
        command_buffer = []
        while True:
            try:
                command_buffer.append(int.from_bytes(self._serial.read(),
                                                     byteorder='big',
                                                     signed=False))
            except serial.SerialException as e:
                # if serial error then try to reconnect
                logger.error('Serial error: %s attempting reconnect...', e)
                self._serial.close()

                backoff = 1
                while not self._serial.is_open:
                    try:
                        self._serial.open()
                    except serial.SerialException as e1:
                        # if reconnect fails then wait and try again
                        logger.error(
                            'Failed to reconnect: %s, waiting %d seconds', e1, backoff)
                        time.sleep(backoff)
                        # incremental backoff
                        backoff += 1
                    continue
                logger.info('Reconnected to serial port')

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
                            if command.command_type in self._command_callbacks:
                                self._command_callbacks[command.command_type](
                                    command)
                            command_buffer = []
                            break
                        except ValueError as e:
                            logger.error('Invalid pkt received: %s', e)
