"""
Parses the firmware file.
"""

import os.path


_DEFAULT_RESULT = {
    'start_address': -1,
    'flash_data': b''
}

_MAX_BYTES_PER_RECORD = 0x10


def read_hex_file(filename):
    if not os.path.isfile(filename):
        raise Exception('The file does not exist.\n{}'.format(filename))

    result = _DEFAULT_RESULT.copy()
    with open(filename, 'r') as fp:
        while True:
            line = fp.readline()
            if line == '':
                break
            # Records must contain a ':'.
            # Note that the record does not have to start with the ':'.
            # It is allowed to precede records with data before the ':'.
            pos = line.find(':')
            if pos < 0:
                # No record
                continue

            # Strip line so we only have the data
            line = line[pos + 1:].strip()
            record_data = bytes.fromhex(line)
            if record_data[3] == 0x00:
                # Data record
                # Check the checksum, if we sum all data including the checksum, the 8-bit result should be zero
                if sum(record_data) & 255 != 0:
                    raise Exception('Checksum error in record:\n{}'.format(line))
                n_bytes = record_data[0]
                if len(record_data) != n_bytes + 5:
                    raise Exception('Record length is not matching the number of bytes.\n{}'.format(line))
                address = record_data[1] * 256 + record_data[2]
                if result['start_address'] < 0:
                    result['start_address'] = address
                if len(result['flash_data']) != address:
                    raise Exception('Address not consistent with the amount of data.\n{}'.format(line))
                result['flash_data'] += record_data[4:-1]

    return result


def write_hex_file(filename, data, start_address):
    address = start_address
    with open(filename, 'w') as fp:
        i = 0
        while i < len(data):
            record = b''
            n_bytes = _MAX_BYTES_PER_RECORD
            if len(data[i:]) < _MAX_BYTES_PER_RECORD:
                n_bytes = len(data[i:])
            record += n_bytes.to_bytes(1, 'big')
            record += (i + address).to_bytes(2, 'big')
            record += b'\x00'
            record += data[i:i + n_bytes]
            crc = (~sum(record) + 1) & 0xFF
            record += crc.to_bytes(1, 'big')
            fp.write(':{}\n'.format(record.hex().upper()))
            i += n_bytes
        # Write end of file record
        fp.write(':00000001FF\n')


if __name__ == '__main__':

    test_filename = '..\\..\\..\\firmware\\test_application\\Debug\\test_application.hex'
    parser_result = read_hex_file(test_filename)
    print(parser_result)

    test_filename = test_filename.replace('.hex', '_1.hex')
    write_hex_file(test_filename, parser_result['flash_data'][:-8], parser_result['start_address'])
