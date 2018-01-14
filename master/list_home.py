#!/usr/bin/python

list_room = [["salon", 1, 10, 0], ["bedroom", 10, 20, 0], ["kitchen", 1, 12, 0], ["bathroom", 5, 30, 0], ["wc", 3, 40, 0], ["office", 3, 50, 0], ["entrance", 1, 5, 0]]



def get_nb_max_room(room):
    for i in list_room :
        if i[0] == room:
            return i[1]
    return -1

def get_begin_addr(room):
    for i in list_room :
        if i[0] == room :
            return i[2]
    return -1

def add_room(room):
    print "add_func"
    for i in list_room :
        print i
        print room
        if i[0] == room :
            print i[0]
            print i[3]
            print i[1]
            if i[3] < i[1] :
                i[3] = i[3] + 1
                return 1
            else :
                return -2
    return -1

def del_room(room):
    for i in list_room :
        if i[0] == room :
            if i[3] > 0:
                i[3] = i[3] - 1
                return 1
            else :
                return -3
    return -1

def get_addr_range(room):
    ans = []
    for i in list_room :
        if i[0] == room :
            j = 1
            ans.append(i[2])
            while j < i[1] :
                ans.append(i[2] + j)
                j = j + 1
    return ans

def get_room_by_addr(addr):
    for i in list_room :
        j = 0
        while j < i[2]:
            if addr == i[1] + j :
                return i[0]
    return "not used"

