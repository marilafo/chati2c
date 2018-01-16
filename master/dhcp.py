#!/usr/bin/python

import smbus

bus = smbus.SMBus(1)

DEVICE_ADDRESS = 0x77


def send_msg(addr, msg1, msg2):
    try:
        bus.write_byte_data(addr, msg1, msg2)
        return 0
    except Exception as e:
        #print (e)
        #print("Write failed")
        return -1

def read_device_answer(addr, n):
    ans=[]
    for i in range (0,n):
        while True:
            try:
                ans.append(bus.read_byte(addr))
                break
            except Exception as e:
                print(e)
                print("Read failed")
    return ans


def check_end_answer(addr, ans,n): 
    print ans
    no_end = 1
    cmpt=0
    if ans[n-1] == 119 :
        print "ok"
        return 0
    ret = -1 
    while no_end == 1:
        try: 
            ret = bus.read_byte(addr);
        except Exception as e:
            print(e)
            print("Read failed")
            cmpt = cmpt + 1
        if ret == 119 or cmpt > 5:
            return 1


def look_for_device():
    while True:
        try:
            bus.write_byte_data(DEVICE_ADDRESS, 200, 200)
            break
        except Exception as e:
            continue
            #print (e)
            #print("Write failed")


def check_dhcp_ans(ans):
    if check_end_answer(DEVICE_ADDRESS, ans,3) == 1 or ans[0] == 119 or ans[1] == 119:
        print "ko"
        return -1
    return 0
    

def send_addr(nb_alea, addr):
    try :
        bus.write_byte_data(DEVICE_ADDRESS, nb_alea , addr)
    except Exception as e:
        print (e)
        print("Write failed")
    
