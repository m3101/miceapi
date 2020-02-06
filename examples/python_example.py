import miceapi
import re

#This is a simple demonstration of the main mouse-focused features of miceapi

#Copyright (c) 2020 Amélia O. F. da S.
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.

#A regex pattern to filter the most common mouse/trackpad/touchpad names
mouser=re.compile(r'ouse|rackpad|ouchpad')
#We get a list of available device names and paths
devices=miceapi.listDevices()

mice=[]
#We add every device that matches our regex pattern to the mice list
for i,device in enumerate(devices):
    if mouser.findall(device[0]):
        mice.append(device)

#Now we transform each member of mice from
#   a (string name,string path) tuple
#   into
#   a (string name,miceapi.Device(path)) tuple
mice=[(mouse[0],miceapi.Device(mouse[1])) for mouse in mice]

#Now that all the mice are configured, we wait for a click on each one.
for mouse in mice:
    print("Please left click on device "+mouse[0])
    mouse[1].wait_mousedown()