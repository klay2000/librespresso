"""entry point for the librespresso backend"""

import argparse
import logging
import signal
import sys

from librespresso import command
from librespresso.config import load_config
from librespresso.heartbeat import Heartbeat
from librespresso.mainloop import MainLoop
from librespresso.uart import Uart

logger = logging.getLogger()
handler = logging.StreamHandler(stream=sys.stdout)
logger.addHandler(handler)


def signal_handler(_0, _1):
    """Handler for SIGINT and SIGTERM signals"""
    logger.info('Exiting...')
    sys.exit(0)


def main():
    """entry point for the librespresso backend"""
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    parser = argparse.ArgumentParser(prog='librespresso',
                                     description="librespresso backend")
    parser.add_argument('--config',
                        help='Path to the configuration file, a file will be created with default '
                        'values if not present',
                        default='config.json')
    parser.add_argument('--nh', '--no-heartbeat',
                        help='Disable automatic heartbeat',
                        action='store_true')
    parser.add_argument('--debug',
                        help='Enable debug logging',
                        action='store_true')

    args = parser.parse_args()

    logger.setLevel(logging.DEBUG if args.debug else logging.INFO)

    config = load_config(args.config)

    logger.info('Loaded configuration: %s', config)
    uart = Uart(config.mcu_serial_port, config.mcu_baudrate)
    heartbeat = Heartbeat(uart, config.heartbeat_interval)

    if args.nh:
        logger.info('Heartbeat disabled')
        heartbeat.stop()
    else:
        @uart.uart_callback(command.CommandType.HB)
        def hb_callback(pkt):
            logger.debug('Heartbeat received: %s', pkt)

    if args.debug:
        @uart.uart_callback(command.CommandType.DEBUG)
        def debug_callback(pkt):
            logger.debug('Debug message received: %s',
                         bytearray(pkt.data).decode())

    mainloop = MainLoop(uart, config)


if __name__ == '__main__':
    main()
