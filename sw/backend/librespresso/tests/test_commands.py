"""Tests for the Command class."""

from librespresso.command import Command, CommandType


def test_hb():
    """Test creating a heartbeat command."""
    b = Command.heartbeat().to_bytes()
    assert b == [0x02, 0x00, 0x00, 0x00, 0x03]
    pkt = Command.from_bytes(b)
    assert pkt.command_type == 0x00
    assert pkt.data == []
    assert pkt.checksum() == 0x00
    assert pkt.to_bytes() == b


def test_checksum():
    """Test calculating the checksum of a command."""
    pkt = Command(CommandType.HB, [0x01, 0x02, 0x03])
    assert pkt.checksum() == 0x06
    pkt = Command(CommandType.DEBUG, [0xDE, 0xAD, 0xBE, 0xEF])
    assert pkt.checksum() == 0x3A
