#!/usr/bin/python

import smbus
import dhcp as dhcp
import list_home as lh

begin_addr = 3
end_addr = 118
broadcast_addr = 119

used_addr = []

code_room = [["salon", 201], ["bedroom", 202], ["kitchen", 203], ["bathroom", 204], ["wc", 205], ["office", 206], ["entrance", 207]]

def dhcp_func() :
    dhcp.look_for_device()
    ans = dhcp.read_device_answer(3)
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
                        used_addr.append = i 
                        break
            break
        else :
            addr = 231 #Le code room n'existe pas
    dhcp.send_addr(ans[0],addr)
    
def main():
    while 1 :
        dhcp_func()

main()    
