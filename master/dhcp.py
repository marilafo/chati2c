#!/usr/bin/python

import smbus

bus = smbus.SMBus(1)

DEVICE_ADDRESS = 0x42


def look_for_device():
    while True:
        try:
            #Message (42,42) pour demander quel est le type de rasp que l'on a;
            bus.write_byte_data(DEVICE_ADDRESS, 200, 200)
            break
        except Exception as e:
            print (e)
            print("Write failed")

def read_device_answer(n):
    ans=[]
    for i in range (0,n):
        while True:
            try:
                ans.append(bus.read_byte(DEVICE_ADDRESS))
                break
            except Exception as e:
                print(e)
                print("Read failed")
    return ans

def check_end_answer(ans,n): 
    print ans
    no_end = 1
    cmpt=0
    if ans[n-1] == 119 :
        print "ok"
        return 0
    while no_end == 1:
        try: 
            ret = bus.read_byte(DEVICE_ADDRESS);
        except Exception as e:
            print(e)
            print("Read failed")
            cmpt = cmpt + 1
        if ret == 119 or cmpt > 5:
            return 1

def check_dhcp_ans(ans):
    if check_end_answer(ans,3) == 1 or ans[0] == 119 or ans[1] == 119:
        print "ko"
        return -1
    return 0
    

def send_addr(nb_alea, addr):
    try :
        bus.write_byte_data(DEVICE_ADDRESS, nb_alea , addr)
    except Exception as e:
        print (e)
        print("Write failed")
    
