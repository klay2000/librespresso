"""Heartbeat class that sends a heartbeat message on a defined interval."""
import threading
import time

from librespresso.command import Command
from librespresso.uart import Uart


class Heartbeat:
    """Heartbeat class that sends a heartbeat message on a defined interval."""

    def __init__(self, uart: Uart, interval: int = 1):
        self._uart = uart
        self._interval = interval
        self._hb_thread = threading.Thread(target=self._hb_thread_fn)
        self._ok = True
        self.start()

    def start(self):
        """Start the heartbeat thread."""
        self._ok = True
        self._hb_thread.start()

    def stop(self):
        """Stop the heartbeat thread."""
        self._ok = False
        self._hb_thread.join()

    def _hb_thread_fn(self):
        """Heartbeat thread that sends a heartbeat message every second."""
        while self._ok:
            # logger.debug('Sending heartbeat...')
            self._uart.send(Command.heartbeat())
            time.sleep(1)
