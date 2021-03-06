#!/usr/bin/python

import smbus
import dhcp as dhcp
import list_home as lh

begin_addr = 3
end_addr = 118
broadcast_addr = 119

addr_broadcast = 0x77

used_addr = []

code_room = [["salon", 201], ["bedroom", 202], ["kitchen", 203], ["bathroom", 204], ["wc", 205], ["office", 206], ["entrance", 207]]

def dhcp_func() :
    dhcp.look_for_device()
    ans = dhcp.read_device_answer(addr_broadcast, 3)
    if dhcp.check_dhcp_ans(ans) == -1:
        dhcp_func()
        return
    for i in code_room:
        if i[1] == ans[1]: 
            room = i[0]
            err = lh.add_room(room)
            if err != 1 :
                addr = 230 #Plus de place libre
            else :
                possible_addr = lh.get_addr_range(room)
                for i in possible_addr :
                    if i in used_addr :
                        continue
                    else:
                        addr = i
                        used_addr.append(i) 
                        break
            break
        else :
            addr = 231 #Le code room n'existe pas
    dhcp.send_addr(ans[0],addr)


def check_slave_ok(addr):
    ok = -1
    while ok == -1:
        ok = dhcp.send_msg(addr, 130, 139)
    ans = dhcp.read_device_answer(addr, 2)
    err = dhcp.check_end_answer(addr, ans, 2)
    if err == 0 :
        if ans[0] == 1:
            return 0
    else:
        return -1




def ask_temperature(addr):
    ok = -1
    while ok == -1:
        ok = dhcp.send_msg(addr, 130, 141)
    ans = dhcp.read_device_answer(addr, 2)
    err = dhcp.check_end_answer(addr, ans, 2)
    if err == 0 :
        return ans[0]
    else:
        return -1

def ask_light(addr):
    ok = -1 
    while ok == -1 :
        ok = dhcp.send_msg(addr, 130, 142)
    ans = dhcp.read_device_answer(addr, 2)
    err = dhcp.check_end_answer(addr, ans, 2)
    if err == 0 :
        return ans[0]
    else:
        return -1

def ask_name(addr):
    ok = -1 
    room = []
    while ok == -1 :
        ok = dhcp.send_msg(addr, 130, 143)
    ans = dhcp.read_device_answer(addr, 2)
    err = dhcp.check_end_answer(addr, ans, 2)
    cmpt = ans[0]
    if err == 0 :
        name = dhcp.read_device_answer(addr, cmpt)
        for i in name :
            room.append(str(unichr(i)))
            
        return room
    else:
        return -1

def shutdown_slave(addr):
    ok = -1
    while ok == -1 :
        ok = dhcp.send_msg(addr, 199, 199)


def ask_slave_request(addr):
    res = -1
    ok = -1 
    while ok == -1:
        ok = dhcp.send_msg(addr, 130, 140)
    ans = dhcp.read_device_answer(addr, 3)
    err = dhcp.check_end_answer(addr, ans, 3)
    if err == 0 :
        if ans[0] == 149:
            return 
        for i in code_room :
            if i[1] == ans[0]:
                room = i[0]
                break
        addr_range = lh.get_addr_range(room)

        for j in addr_range:
            if j in used_addr :
                if ans[1] == 141 :
                    while res == -1 :
                        res = ask_temperature(j)
                elif ans[1] == 142 :
                    while res == -1:
                        res = ask_light(j)  
        dhcp.send_msg(addr, 145, res)
    else:
        dhcp.send_msg(addr, 145, 144)





def main():
    while 1 :
        dhcp_func()

