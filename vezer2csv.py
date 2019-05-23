#!/usr/bin/env python3

import sys
from collections import defaultdict
import xml.etree.ElementTree as ET

class BaseElement:
    def __init__(self, element):
        self.element = element

    def value(self, path, t):
        return t(self.element.find(path).text)

    
class Composition(BaseElement):
    def __init__(self, element):
        super().__init__(element)

        self.fps = self.value('fps', int)
        self.length = self.value('length', int)
        self.start = self.value('start', int)
        self.end = self.value('end', int)
        self.name = self.value('name', str)
        
        print(self)
        print(f'  {self.length} frames, start {self.start}, end {self.end}')
        
        track_elements = list(self.element.findall('tracks/track'))
        print(f'  found {len(track_elements)} tracks, adding:')
        self.tracks = []
        self.num_channels = 0
        skipped = []
        
        for te in track_elements:
            track = Track(te)
            if track.is_dmx():
                track.process()
                self.tracks.append(track)
                print(f'    {track}')
                max_chan = max(track.channel_numbers())
                self.num_channels = max(self.num_channels, max_chan)
            else:
                skipped.append(track)

        if len(skipped) > 0:
            print(f'  skipping: {", ".join([s.name for s in skipped])}')
                
        print(f'  composition has {self.num_channels} DMX channels')
                
    def export_csv(self, prefix=''):
        slug = self.name.lower().replace(' ', '_')
        filename = f'{prefix}{slug}_{self.fps}fps.csv'
        print(f'  saving to {filename}')
        
        with open(filename, 'w') as f:
            for frame_number in range(self.start, self.end + 1):
                row = ['0' for _ in range(self.num_channels)]
                for track in self.tracks:
                    values = track.values(frame_number)
                    for chan, value in values.items():
                        row[chan - 1] = str(value)
                f.write(f'{",".join(row)}\n')
            
                
    def __str__(self):
        return f'composition {self.name} at {self.fps}fps'

    
class Track(BaseElement):
    def __init__(self, element):
        super().__init__(element)
        self.name = self.value('name', str)
        
        if self.is_dmx():
            self.base_channel = self.value('target/channels', int)
            res = self.value('target/resolution', str)
            self.is16bit = res == '16bit'

    def is_dmx(self):
        return self.value('type', str) == 'ArtNetValue'

    def process(self):
        frames = list(self.element.find('process'))

        self.num_frames = len(frames)
        self.channels = {}
        self.channels[self.base_channel] = [0 for _ in frames]
        if self.is16bit:
            self.channels[self.base_channel + 1] = [0 for _ in frames]
        
        for frame in frames:
            fn = int(frame.tag[1:]) # remove f prefix
            value = int(frame.text)
            if self.is16bit:
                self.channels[self.base_channel][fn] = value >> 8
                self.channels[self.base_channel + 1][fn] = value & 0xff
            else:
                self.channels[self.base_channel][fn] = value
                
    def values(self, frame_number):
        try: 
            return { c: frames[frame_number]
                     for c, frames in self.channels.items() }
        except IndexError:
            return { c: 0 for c in self.channels.keys() }

    def channel_numbers(self):
        return self.channels.keys()

    def __str__(self):
        channels = [str(k) for k in self.channel_numbers()]
        chan = f'channel{"s" if len(channels) > 1 else ""}'
        extra = '(16bit)' if self.is16bit else ''
        return f'track {self.name} {extra} on {chan} {" & ".join(channels)} ({self.num_frames} frames)'
        

if __name__ == "__main__":
    try:
        filename = sys.argv[1]
    except IndexError:
        print('please specify a vezer export')
        exit()

    
    tree = ET.parse(filename)
    root = tree.getroot()

    for ce in root.findall('composition'):
        composition = Composition(ce)
        composition.export_csv()
        print()
