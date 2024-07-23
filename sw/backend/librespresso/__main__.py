"""entry point for the librespresso backend"""

import argparse
import logging
import signal
import sys

from librespresso import command
from librespresso.config import load_config
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
    parser.add_argument('--command',
                        help='Command mode, send raw commands and receive responses',
                        action='store_true')
    parser.add_argument('--heartbeat',
                        help='Automatically send heartbeat messages in command mode',
                        action='store_true')
    parser.add_argument('--debug',
                        help='Enable debug logging',
                        action='store_true')

    args = parser.parse_args()

    logger.setLevel(logging.DEBUG if args.debug else logging.INFO)

    config = load_config(args.config)

    logger.info('Loaded configuration: %s', config)
    if args.command:
        uart = Uart(config.mcu_serial_port, config.mcu_baudrate)
        if args.heartbeat:
            logger.info('Starting heartbeat...')
            uart.start_heartbeat()

            @uart.uart_callback(command.CommandType.HB)
            def hb_callback(pkt):
                logger.debug('Heartbeat received: %s', pkt)

        @uart.uart_callback(command.CommandType.DEBUG)
        def debug_callback(pkt):
            logger.debug('Debug message received: %s',
                         bytearray(pkt.data).decode())

        logger.info('Entering command mode...')
        while True:
            pass
            # try:
            #     command_str = input('Enter command: ')
            #     command_bytes = bytes.fromhex(command_str)
            #     uart.send(command.Command.from_bytes(command_bytes))
            # except Exception as e:
            #     logger.error('Error sending command: %s', e)


if __name__ == '__main__':
    main()
