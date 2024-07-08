"""entry point for the librespresso backend"""

import argparse
from config import load_config
import command
import logging
import sys
import UART
import signal

# TODO: setup some tooling for autoformatting and linting python

logger = logging.getLogger()
handler = logging.StreamHandler(stream=sys.stdout)
logger.addHandler(handler)

def signal_handler(sig, frame):
    logger.info('Exiting...')
    sys.exit(0)

def main():
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    parser = argparse.ArgumentParser(prog='librespresso', description="librespresso backend")
    parser.add_argument('--config', help='Path to the configuration file, a file will be created with default values if not present', default='config.json')
    parser.add_argument('--command', help='Command mode, send raw commands and receive responses', action='store_true')
    parser.add_argument('--heartbeat', help='Automatically send heartbeat messages in command mode', action='store_true')
    parser.add_argument('--debug', help='Enable debug logging', action='store_true')

    args = parser.parse_args()

    logger.setLevel(logging.DEBUG if args.debug else logging.INFO)

    config = load_config(args.config)

    logger.info(f'Loaded configuration: {config}')
    if args.command:
        uart = UART.UART(config.mcu_serial_port, config.mcu_baudrate)
        if args.heartbeat:
            logger.info('Starting heartbeat...')
            uart.start_heartbeat()
    
            @UART.UART.UART_callback(command.CommandType.HB)
            def hb_callback(pkt):
                pass
                # logger.debug(f'Heartbeat received: {pkt}')
            
        @UART.UART.UART_callback(command.CommandType.DEBUG)
        def debug_callback(pkt):
            logger.debug(f'Debug message received: {bytearray(pkt.data).decode()}')
        
        logger.info('Entering command mode...')
        while True:
            try:
                command_str = input('Enter command: ')
                command_bytes = bytes.fromhex(command_str)
                uart.send(command.Command.from_bytes(command_bytes))
            except Exception as e:
                logger.error(f'Error sending command: {e}')
        

if __name__ == '__main__':
    main()