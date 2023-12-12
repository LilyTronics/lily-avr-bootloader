"""
Generate random data for the test application firmware.
"""

import random


N_BYTES = 400

output = []
while N_BYTES > 0:
    output.append('0x{:02X}'.format(random.randint(0, 255)))
    if len(output) == 10:
        print('    {},'.format(', '.join(output)))
        del output[:]
    N_BYTES -= 1

if len(output) > 0:
    print('    {}'.format(', '.join(output)))
    del output[:]
