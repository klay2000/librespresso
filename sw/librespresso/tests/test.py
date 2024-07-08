import pytest
from sw.librespresso.command import Communication

def test_hb():
    b = Communication.Command.heartbeat().to_bytes()
    assert b == [0x02, 0, 0x02, 0, 0x03]
    pkt = Communication.Command.from_bytes(b)
    assert pkt.command_type == 0
    assert pkt.data == []
    assert pkt.checksum() == 0x02
    assert pkt.to_bytes() == b