"""
Contains all function for the bootloader.
"""

import serial
import time


class BootLoader(object):

    _CMD_START = 0x01

    _CMD_ACTIVATE = 0x02
    _CMD_DEACTIVATE = 0x03
    _CMD_VERSION = 0x04

    _CMD_DEVICE_NAME = 0x10
    _CMD_MODULE_NAME = 0x11

    _CMD_BOOT_SIZE = 0x20
    _CMD_FLASH_SIZE = 0x21
    _CMD_PAGE_SIZE = 0x22

    _CMD_SET_PAGE_ADDRESS = 0x30
    _CMD_READ_PAGE = 0x31
    _CMD_WRITE_PAGE = 0x32

    _CMD_ERROR = 0xFF

    _RX_TIME_OUT = 3
    _MAX_DEBUG_DATA_LENGTH = 20

    def __init__(self, port, speed, show_debug=False):
        self._show_debug = show_debug
        self._serial = serial.Serial(port, speed)
        if self._show_debug:
            print('Serial port:', self._serial)

    def __del__(self):
        try:
            self._serial.close()
        except (Exception, ):
            pass

    ###########
    # Private #
    ###########

    @ staticmethod
    def _invert_byte(byte):
        return ~byte + 256

    def _packet_to_hex_string(self, packet):
        output = ' '.join(map(lambda x: '0x{:02X}'.format(x), packet[:4 + self._MAX_DEBUG_DATA_LENGTH]))
        if len(packet[4:]) > self._MAX_DEBUG_DATA_LENGTH:
            output += ' ... ({} data bytes)'.format(len(packet[4:]))
        return output

    def _show_tx_packet_data(self, packet):
        if self._show_debug:
            print('TX -> {}'.format(self._packet_to_hex_string(packet)))

    def _show_rx_packet_data(self, packet):
        if self._show_debug:
            print('RX <- {}'.format(self._packet_to_hex_string(packet)))

    def _send_receive(self, command, data=b''):
        packet = b''
        packet += self._CMD_START.to_bytes(1, 'big')
        packet += command.to_bytes(1, 'big')
        packet += len(data).to_bytes(2, 'big')
        packet += data
        self._show_tx_packet_data(packet)
        self._serial.write(packet)

        start = time.time()
        rx_data = b''
        while (time.time() - start) < self._RX_TIME_OUT:
            while self._serial.in_waiting > 0:
                rx_data += self._serial.read(self._serial.in_waiting)

            # Analyse packet
            if len(rx_data) >= 4 and rx_data[0] == self._CMD_START:
                n_bytes = rx_data[2] + rx_data[3] + 4
                if len(rx_data) == n_bytes:
                    self._show_rx_packet_data(rx_data)
                    break

            time.sleep(0.1)

        else:
            raise Exception('RX timeout')

        if rx_data[1] == self._CMD_ERROR:
            raise Exception('Error when sending command: 0x{:02X}'.format(command))

        return rx_data

    def activate(self):
        result = self._send_receive(self._CMD_ACTIVATE)
        return self._invert_byte(result[1]) == self._CMD_ACTIVATE

    def deactivate(self):
        result = self._send_receive(self._CMD_DEACTIVATE)
        return self._invert_byte(result[1]) == self._CMD_DEACTIVATE

    def get_version(self):
        result = self._send_receive(self._CMD_VERSION)
        if self._invert_byte(result[1]) == self._CMD_VERSION and len(result) == 5:
            return result[4]

        return 0

    def get_device_name(self):
        name = ''
        result = self._send_receive(self._CMD_DEVICE_NAME)
        if self._invert_byte(result[1]) == self._CMD_DEVICE_NAME and len(result) > 4:
            name = result[4:]
        return name

    def get_module_name(self):
        name = ''
        result = self._send_receive(self._CMD_MODULE_NAME)
        if self._invert_byte(result[1]) == self._CMD_MODULE_NAME and len(result) > 4:
            name = result[4:]
        return name

    def get_boot_size(self):
        size = 0
        result = self._send_receive(self._CMD_BOOT_SIZE)
        if self._invert_byte(result[1]) == self._CMD_BOOT_SIZE and len(result) == 6:
            size = result[4] * 256 + result[5]
        return size

    def get_flash_size(self):
        size = 0
        result = self._send_receive(self._CMD_FLASH_SIZE)
        if self._invert_byte(result[1]) == self._CMD_FLASH_SIZE and len(result) == 6:
            size = result[4] * 256 + result[5]
        return size

    def get_page_size(self):
        size = 0
        result = self._send_receive(self._CMD_PAGE_SIZE)
        if self._invert_byte(result[1]) == self._CMD_PAGE_SIZE and len(result) == 6:
            size = result[4] * 256 + result[5]
        return size

    def set_page_address(self, address):
        result = self._send_receive(self._CMD_SET_PAGE_ADDRESS, address.to_bytes(4, 'big'))
        return self._invert_byte(result[1]) == self._CMD_SET_PAGE_ADDRESS

    def read_page(self):
        data = b''
        result = self._send_receive(self._CMD_READ_PAGE)
        if self._invert_byte(result[1]) == self._CMD_READ_PAGE and len(result) > 4:
            data = result[4:]
        return data

    def write_page(self, page_data):
        result = self._send_receive(self._CMD_WRITE_PAGE, page_data)
        return self._invert_byte(result[1]) == self._CMD_WRITE_PAGE


if __name__ == '__main__':

    import random

    _MAX_DATA_IN_STRING = 20

    def _data_to_string(data):
        output = ' '.join(map(lambda x: '0x{:02X}'.format(x), data[:_MAX_DATA_IN_STRING]))
        if len(data) > _MAX_DATA_IN_STRING:
            output += ' ... ({} data bytes)'.format(len(data))
        return output

    def _test_command(label, command, param=None, expect_pass=True):
        print('\n{} . . .'.format(label))
        try:
            if param is None:
                result = command()
            else:
                result = command(param)
            if type(result) is bytes:
                result = _data_to_string(result)
            print('{:17}:'.format(label), result)
            return 0
        except Exception as e:
            if expect_pass:
                print('\nERROR: {}'.format(e))
                return 1
            else:
                print('{:17}: failed as expected'.format(label))
                return 0


    n_failed = 0

    boot_loader = BootLoader('COM19', 115200, True)
    time.sleep(1)

    n_failed += _test_command('Activate', boot_loader.activate)
    time.sleep(1)
    n_failed += _test_command('Deactivate', boot_loader.deactivate)
    time.sleep(1)
    n_failed += _test_command('Version', boot_loader.get_version, expect_pass=False)
    n_failed += _test_command('Activate', boot_loader.activate)
    n_failed += _test_command('Version', boot_loader.get_version)

    n_failed += _test_command('Device name', boot_loader.get_device_name)
    n_failed += _test_command('Module name', boot_loader.get_module_name)

    n_failed += _test_command('Get boot size', boot_loader.get_boot_size)
    n_failed += _test_command('Get flash size', boot_loader.get_flash_size)
    n_failed += _test_command('Get page size', boot_loader.get_page_size)

    n_failed += _test_command('Set page address', boot_loader.set_page_address, 0x0)
    n_failed += _test_command('Read page', boot_loader.read_page)

    test_data = b'\xff\xcf'
    while len(test_data) < _MAX_DATA_IN_STRING:
        test_data += random.randint(0, 255).to_bytes(1, 'big')

    n_failed += _test_command('Write page', boot_loader.write_page, test_data)

    n_failed += _test_command('Read page', boot_loader.read_page)
    print('Programmed data  :', _data_to_string(test_data))

    n_failed += _test_command('Deactivate', boot_loader.deactivate)

    if n_failed > 0:
        print('\nERROR: one or more tests failed')

    print('\nDone')
