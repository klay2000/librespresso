# TL;DR
``0x02 | (1 byte) length | (1 byte) data checksum | (1 byte) packet id | (0-255 bytes) data | 0x03``

# Protocol
The UART communications between the host pc and microcontroller are based on a simple packet protocol. Each packet is framed by a start byte (0x02) and an end byte (0x03). The packet structure, shown above, consists of an id, data, a simple checksum, and a length (just data length).

## Checksum
The checksum is calculated by summing all the bytes in the data and the packet id and taking the lower 8 bits of the result.

## Packet ID
The packet id is used to identify the type of packet being sent. The id is a single byte and is used to determine how the data should be interpreted. There are 256 possible packet ids, but only some are currently used. The currently assigned packet ids are seen in ``packet.h``.

## Heartbeat
The heartbeat packet is used to ensure that the connection between the host pc and the microcontroller is still active. The heartbeat packet is sent by host pc to the microcontroller and the microcontroller will respond with an identical heartbeat packet. The heartbeat packet has an id of 0x00 and no data. This also has the effect of re-synchronizing the packet framing such that a failed packet will not cause the rest of the packets to be misaligned.

## Packet Types in use
|ID | Name| Description
-|-|-
|0x00 | HB | Heartbeat, must be sent on some interval by the SOC to the MCU. MCU will send back. |
|0x01 | CONFIG | Sent with a 1 byte var identifier and a 4 byte value to set or a 1 byte identifier without a value to read |
|0x02 | DEBUG | Sent only by the MCU to the SOC, contains a string of debug information |