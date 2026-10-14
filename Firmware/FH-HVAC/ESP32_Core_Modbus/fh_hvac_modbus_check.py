#!/usr/bin/env python3
"""FH-HVAC Phase B check: talk to the node over Modbus TCP, standard library only.

Usage (laptop on the same network as the node):
  python fh_hvac_modbus_check.py                    read everything once
  python fh_hvac_modbus_check.py --watch            read every 2 s (Ctrl+C to stop)
  python fh_hvac_modbus_check.py --target 23.0      write TARGET_TEMP_C (HR0), degrees C
  python fh_hvac_modbus_check.py --enable 0         clear HVAC_ENABLE (Coil0); --enable 1 sets it
Options: --ip 192.168.50.21  --port 502  --unit 1
"""
import argparse, socket, struct, time

_tid = 0

def _request(sock, unit, fc, payload):
    global _tid
    _tid = (_tid + 1) & 0xFFFF
    pdu = bytes([fc]) + payload
    sock.sendall(struct.pack(">HHHB", _tid, 0, len(pdu) + 1, unit) + pdu)
    hdr = _recv(sock, 7)
    _, _, length, _ = struct.unpack(">HHHB", hdr)
    body = _recv(sock, length - 1)
    if body[0] == fc | 0x80:
        raise RuntimeError(f"Modbus exception {body[1]} on function {fc}")
    return body[1:]

def _recv(sock, n):
    data = b""
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            raise ConnectionError("connection closed")
        data += chunk
    return data

def read_bits(sock, unit, fc, addr, count):
    resp = _request(sock, unit, fc, struct.pack(">HH", addr, count))
    return [bool(resp[1 + i // 8] >> (i % 8) & 1) for i in range(count)]

def read_regs(sock, unit, fc, addr, count):
    resp = _request(sock, unit, fc, struct.pack(">HH", addr, count))
    return list(struct.unpack(">" + "H" * count, resp[1:1 + 2 * count]))

def write_coil(sock, unit, addr, value):
    _request(sock, unit, 5, struct.pack(">HH", addr, 0xFF00 if value else 0))

def write_reg(sock, unit, addr, value):
    _request(sock, unit, 6, struct.pack(">HH", addr, value))

def show(sock, unit):
    ir = read_regs(sock, unit, 4, 0, 4)
    di = read_bits(sock, unit, 2, 0, 2)
    hr = read_regs(sock, unit, 3, 0, 1)
    co = read_bits(sock, unit, 1, 0, 1)
    print(f"TEMP_C={ir[0]/10:.1f} C (raw {ir[0]})  HUMIDITY_PCT={ir[1]}  HEARTBEAT={ir[2]}  RECONNECT_COUNT={ir[3]}")
    print(f"COMPRESSOR_RUNNING={int(di[0])}  HVAC_FAULT={int(di[1])}  "
          f"TARGET_TEMP_C={hr[0]/10:.1f} C (raw {hr[0]})  HVAC_ENABLE={int(co[0])}")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ip", default="192.168.50.21")
    ap.add_argument("--port", type=int, default=502)
    ap.add_argument("--unit", type=int, default=1)
    ap.add_argument("--target", type=float, help="write TARGET_TEMP_C in degrees C")
    ap.add_argument("--enable", type=int, choices=[0, 1], help="write HVAC_ENABLE")
    ap.add_argument("--watch", action="store_true")
    a = ap.parse_args()
    with socket.create_connection((a.ip, a.port), timeout=5) as s:
        s.settimeout(5)
        if a.target is not None:
            write_reg(s, a.unit, 0, int(round(a.target * 10)))
            print(f"wrote TARGET_TEMP_C = {a.target:.1f} C")
        if a.enable is not None:
            write_coil(s, a.unit, 0, bool(a.enable))
            print(f"wrote HVAC_ENABLE = {a.enable}")
        if a.target is not None or a.enable is not None:
            time.sleep(0.3)
        while True:
            show(s, a.unit)
            if not a.watch:
                break
            print("-")
            time.sleep(2)

if __name__ == "__main__":
    main()
