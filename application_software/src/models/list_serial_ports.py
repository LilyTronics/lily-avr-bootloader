"""
Lists all the available serial ports on the system.
"""

import serial
import threading
import time


def get_available_serial_ports():
    ports = []
    threads = []
    lock = threading.RLock()

    for i in range(1, 256):
        port = 'COM%d' % i
        t = threading.Thread(target=_check_serial_port, args=(lock, port, ports))
        t.daemon = True
        t.start()
        threads.append(t)

    while True in list(map(lambda x: x.is_alive(), threads)):
        time.sleep(0.1)

    if len(ports) == 0:
        ports.append('no ports')

    return sorted(ports)


def get_available_baud_rates():
    return [2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 76800, 115200, 230400, 250000, 500000, 1000000]


def _check_serial_port(lock_object, port_name, port_list):
    p = None
    try:
        p = serial.Serial(port_name)
        lock_object.acquire()
        try:
            port_list.append(port_name)
        finally:
            lock_object.release()
    except (Exception, ):
        pass

    if p is not None:
        p.close()


if __name__ == '__main__':

    _ports = get_available_serial_ports()
    print('\n', _ports)

    print('\nDone')
