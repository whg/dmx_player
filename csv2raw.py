#!/usr/bin/env python3

import sys
from array import array
import struct
from itertools import chain
import re

def create_raw(csv_filename):
    with open(csv_filename) as f:
        raw_lines = f.readlines()

    print(f'opened {csv_filename}')
        
    lines = [l.strip().split(',') for l in raw_lines]

    def process_line(line):
        # so we remove last , if there is one
        parts = filter(None, line)
        return list(map(int, parts))


    values = list(map(process_line, lines))

    nrows = len(values)
    nchannels = len(values[0])
    version = 2
    magic = 10 << 4 | version

    print(f'  found {nchannels} channels and {nrows} frames')
    
    frs = re.findall('(\d+)fps', csv_filename)
    if frs:
        framerate = int(frs[0])
        print(f'  found {framerate}fps')
    else:
        framerate = 25
        print(f'  framerate not in filename, defaulting to {framerate}')


    print(f'  using version {version}')
    
    header = struct.pack('<BIHB', magic, nrows, nchannels, framerate)

    data = nrows * nchannels

    raw_data = array('B', list(chain(*values)))

    output_filename = filename.replace('.csv', '.raw')
    with open(output_filename, 'wb') as f:
        f.write(header)
        raw_data.tofile(f)

    print(f'  saved {output_filename}')


if __name__ == '__main__':
    filenames = sys.argv[1:]

    for filename in filenames:
        create_raw(filename)
    
    
