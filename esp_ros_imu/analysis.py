import serial
import serial.tools.list_ports
import os
import threading
from tkinter import messagebox
import time
import array
from tkinter import *
import pyqtgraph as pg
import numpy as np
from scipy.fftpack import fft
import socket
lock = threading.RLock()
ser = None
has_data = False

import socket
import sys
import time
import threading
import struct


class Node():
    def __init__(self, name, ip, port, f):
        self.name = name
        self.neighbors = {}
        self.lock = threading.Lock()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET,socket.SO_BROADCAST,1)#(SOL_SOCKET, SO_BROADCAST, 1)
        self.sock.bind((ip, port))
        # self.sock.settimeout(1)
        # self.sock.setblocking(0)
        self.broadcast_ip = '<broadcast>'
        
        self.process_function = f
        self.port = port
        self.ip = ip
        self.broadcast_thread = threading.Thread(
            target=self.broadcast, args=())
        self.receive_thread = threading.Thread(target=self.receive, args=())
        self.broadcast_thread.daemon = True
        self.receive_thread.daemon = True
        self.broadcast_thread.start()
        self.receive_thread.start()

    def broadcast(self):
        # find the neighbors and send the message to them
      while True:
        self.lock.acquire()
        msg = "Node:" + self.name + "," + str(self.ip)+"," + str(self.port)
        try:
            self.sock.sendto(msg.encode(), (self.broadcast_ip, self.port))
        except socket.error as e:
            print(f"Error occured while sending data: {e}")
        self.lock.release()
        time.sleep(2)

    def receive(self):
        # try:
            while True:
    
                data, addr = self.sock.recvfrom(1024)
                self.process_function(data, addr)
                time.sleep(0.001)
        # except socket.error:
        #     pass




def get_ip():

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(('8.8.8.8', 80))
    print(s.getsockname()[0])
    str = s.getsockname()[0]
    s.close()
    return str



def ser_init(com=None):
    global ser
    com_num = 0
    port_list = list(serial.tools.list_ports.comports())
    target_com = ''
    if not len(port_list):
        print('无可用串口')
    else:
        print('可用串口列表：', port_list)
        for i in range(len(port_list)):
            port_list[i] = str(port_list[i])
            if "USB 串行设备" in port_list[i]:
                print(port_list[i])
                com_num += 1
                target_com = port_list[i].split(' ')[0]
    try:
        if com_num == 1:
            ser = serial.Serial(target_com, 115200, timeout=2)
            print('已连接到', target_com)
        else:
            if com != None:
                ser = serial.Serial(com, 115200, timeout=2)
                print('已连接到', com)
            else:
                messagebox.showinfo('error:', '请指定串口')
    # 打印错误
    except Exception as e:
        print(e)
        print('串口{}打开失败'.format(target_com))


def serialread():
    global data_line, data_tosave, ser
    nowti = time.time()

    if ser != None:
        while 1:
            try:
                read_gap = time.time()-nowti
                nowti = time.time()
                lock.acquire()
                temp = ser.readline()
                lock.release()
                try:
                
                        data_line = eval(temp)
                        has_data = True
                except:
                    pass
                print(temp.decode('utf-8'))
                # print(str(temp))
                # print(expect,measure)
                del temp
                # var.set(ser.read_all())
            except:
                pass


once = 0
new_data = False
data_win = []

def process_fun(data, addr):
    # print(data,addr
    global data_line, data_tosave, once, new_data, has_data,data_win
    global recive_data, data_line
 
    # print(data)
    if not data.startswith(b'Node'):
            # data_line = eval(data)
            data_line = struct.unpack('BBIffffffffffII', data)[-6:-2]
            s=str(data_line)
            data_win.append(data_line)
            # if(len(data_win)>100):
            #     data_win.pop(0)
            #     std=np.std(data_win,axis=0)
            #     mean=np.mean(data_win,axis=0)
            #     s+=f"std:{std},mean:{mean}"
            print(s)
            has_data = True
    #data = struct.unpack('fffffffff', data)
    # print(data, addr)


# def recive():
#     global data_line, data_tosave, once, new_data, has_data
#     global recive_data, data_line
#     # 创建一个socket,实时接收数据，保存到data_line
#     s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#     s.bind(('0.0.0.0', 8888))
#     datawin=[]
#     print('Waiting for connection...')
#     while True:
#         data, address = s.recvfrom(1024)
#         print(data)
#         if not data.startswith(b'Node'):
#             data_line = eval(data)
#             datawin.append(data_line[1])
#         if(len(datawin)>100):
#             datawin.pop(0)
#             std=np.std(datawin,axis=0)
#             mean=np.mean(datawin,axis=0)
#             print(std,'\t',mean,end='\t ')
#         has_data = True
        # print(data.decode('utf-8'))
class curve_data():
    def __init__(self, color, length, p):
        self.data = array.array('d')
        self.historyLength = length
        self.curve = p.plot(pen=color)

    def append(self, value):
        self.data.append(value)
        if len(self.data) > self.historyLength:
            del self.data[0]
        self.curve.setData(self.data)

    def clear(self):
        self.data = array.array('d')
        self.curve.setData(self.data)

    def append_fft(self, value):
        self.data.append(value)
        if len(self.data) > self.historyLength:
            del self.data[0]
        fft_data = fft(self.data)
        self.curve.setData(np.abs(fft_data)/512)


def drawgrp():
    global ser, data_line
    app = pg.mkQApp()  # 建立app
    pg.setConfigOption('background', 'w')
    win = pg.GraphicsWindow()  # 建立窗口
    win.setWindowTitle(u'串口绘图器')
    win.resize(800, 500)  # 小窗口大小

    data = array.array('d')  # 可动态改变数组的大小,double型数组
    historyLength = 512  # 横坐标长度
    p = win.addPlot()  # 把图p加入到窗口中
    p.showGrid(x=True, y=True)  # 把X和Y的表格打开

    p.setLabel(axis='left', text='y / V')  # 靠左
    p.setLabel(axis='bottom', text='x / point')
    p.setTitle('串口绘图器')  # 表格的名字
    c1 = curve_data('b', historyLength, p)
    c2 = curve_data('r', historyLength, p)

    c_list = [c1, c2]
    t1, t2 = 0, 0

    def plotData():

        global ser, data_line
        if not has_data:
            return
        t1 = time.time()
        if len(c_list) != len(data_line):
            color_list = ['r', 'g', 'b', 'y', 'c', 'm', 'w',
                          'k', 'r', 'g', 'b', 'y', 'c', 'm', 'w', 'k']
            c_list.clear()
            for i in range(len(data_line)):
                c_list.append(curve_data(color_list[i], historyLength, p))
        # c_list[2].append_fft(np.sqrt(data_line[0]**2+data_line[1]**2+data_line[2]**2))
        for i in range(len(data_line)):
            c_list[i].append(data_line[i])
        app.processEvents()  # 不断刷新
        t2 = time.time()
    timer = pg.QtCore.QTimer()
    timer.timeout.connect(plotData)  # 定时调用plotData函数
    timer.start(2)  # 多少ms调用一次
    app.exec_()


use_ser = False
use_net = True

threading.Thread(target=drawgrp).start()
if use_ser:
    ser_init("COM3")
    t1 = threading.Thread(target=serialread)
    t1.start()
if use_net:
    node = Node('process_node', get_ip(),18688, process_fun)



