"""
Parses the firmware file.
"""

import os.path


_DEFAULT_RESULT = {
    'start_address': -1,
    'flash_data': b''
}


def parse_file(filename):
    if not os.path.isfile(filename):
        raise Exception('The file does not exist.\n{}'.format(filename))
    if filename.endswith('.hex'):
        result = _parse_hex_file(filename)
    else:
        raise Exception('The file format is not supported.')

    return result


def _parse_hex_file(filename):
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
                    raise Exception('Check sum error in record:\n{}'.format(line))
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


if __name__ == '__main__':

    files_to_parse = [
        '..\\..\\..\\firmware\\test_application\\Debug\\test_application.hex'
    ]

    for file_to_parse in files_to_parse:
        parser_result = parse_file(file_to_parse)
        print(parser_result)
