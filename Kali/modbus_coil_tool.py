"""Tiny Modbus TCP coil tool for the CyberCity lab (own PLC containers only).
Standard library only, no install needed. Written Oct 28, 2026; NOT run by the agent.
Usage:
  python3 modbus_coil_tool.py read  HOST ADDRESS COUNT [UNIT]
  python3 modbus_coil_tool.py write HOST ADDRESS 0|1   [UNIT]
Examples (container BSH-PowerBox, its own coils %QX0.0 to %QX0.5 = coils 0 to 5):
  python3 modbus_coil_tool.py read  192.168.50.114 0 6
  python3 modbus_coil_tool.py write 192.168.50.114 0 1
"""
import socket
import struct
import sys


def txn(host, pdu, unit, tid=1, port=502):
    frame = struct.pack(">HHHB", tid, 0, len(pdu) + 1, unit) + pdu
    with socket.create_connection((host, port), timeout=3) as s:
        s.sendall(frame)
        data = s.recv(260)
    if len(data) < 9:
        raise SystemExit("short reply: %r" % data)
    if data[7] & 0x80:
        raise SystemExit("Modbus exception code %d" % data[8])
    return data[8:]


def main():
    a = sys.argv
    if len(a) < 5 or a[1] not in ("read", "write"):
        raise SystemExit(__doc__)
    host, addr, arg = a[2], int(a[3]), int(a[4])
    unit = int(a[5]) if len(a) > 5 else 0
    if a[1] == "read":
        body = txn(host, struct.pack(">BHH", 1, addr, arg), unit)
        for i in range(arg):
            bit = (body[1 + i // 8] >> (i % 8)) & 1
            print("coil %d = %d" % (addr + i, bit))
    else:
        val = 0xFF00 if arg else 0x0000
        body = txn(host, struct.pack(">BHH", 5, addr, val), unit)
        print("wrote coil %d = %d (echo %s)" % (addr, 1 if arg else 0, body.hex()))


main()
