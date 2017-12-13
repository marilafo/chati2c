#!/usr/bin/python

import smbus

bus = smbus.SMBus(1)

DEVICE_ADDRESS = 0x42
toto=[]

while True:
    try:
        bus.write_byte_data(DEVICE_ADDRESS, 15, 12)
        break
    except Exception as e:
        print (e)
        print("Write failed")

for i in range (0,2):
    while True:
        try:
            toto.append(bus.read_byte(DEVICE_ADDRESS))
            break
        except Exception as e:
            print(e)
            print("Read failed")
print toto
