"""Class that initializes the system then polls to update the system state."""
import logging
import struct
import threading
import time

from librespresso.command import Command, ConfigParameter
from librespresso.config import Config, PIDParams, LinearCalParams
from librespresso.uart import Uart

logger = logging.getLogger(__name__)


class MainLoop:
    """Main loop for the system."""

    def __init__(self, uart: Uart, config: Config):
        """Initialize the system."""
        # self._state = state.State()
        self._uart = uart
        self._config = config
        self._configure()
        self._polling_thread = threading.Thread(target=self._polling_thread_fn)
        self._polling_thread.start()

    def _configure(self):
        """Configure the system."""

        logger.info('restarting mcu...')
        self._uart.send(Command.reset())
        time.sleep(2)

        logger.info('configuring mcu...')

        # configure calibration for the pressure sensor
        self._config_linear_cal(self._config.pump_pressure_calibration,
                                ConfigParameter.PUMP_PRESSURE_CAL)

        # configure pid loop for the pump
        self._configure_pid(self._config.pump_pid, ConfigParameter.PUMP_PID)

        logger.info('mcu configured, continuing...')
        self._uart.send(Command.start())

    def _polling_thread_fn(self):
        """Main loop that polls the system state."""
        while True:
            time.sleep(1)

    def _config_linear_cal(self, cal_params: LinearCalParams, param: ConfigParameter):
        """Configure parameters for a linear calibration."""
        self._configure_struct(param, 'ff', cal_params.m, cal_params.b)

    def _configure_pid(self, pid_params: PIDParams, param: ConfigParameter):
        """Configure a PID loops parameters."""
        self._configure_struct(param, 'fffff', pid_params.p,
                               pid_params.i, pid_params.d, pid_params.out_min, pid_params.out_max)

    def _configure_float(self, value: float, param: ConfigParameter):
        """Configure a float parameter."""
        self._configure_struct(param, 'f', value)

    def _configure_struct(self, param: ConfigParameter, struct_format: str, *args):
        """Configure a struct parameter."""
        data_bytes = struct.pack(struct_format, *args)
        pkt = Command.config(param, data_bytes)
        self._uart.send(pkt)
