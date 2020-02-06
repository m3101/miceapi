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
import threading
def _wait_move_async(device,callback,args):
    ret=False
    while not ret:
        move=device.wait_move()
        ret=callback(move,*args)
    return 0
def on_move(device,callback,args=()):
    threading.Thread(target=_wait_move_async,args=(device,callback,args)).start()
    return 0
def _wait_mousedown_async(device,callback,args,params):
    ret=False
    while not ret:
        down=device.wait_mousedown(*params)
        ret=callback(down,*args)
    return 0
def on_mousedown(device,callback,args=(),params=(1,0,0)):
    threading.Thread(target=_wait_mousedown_async,args=(callback,params)).start()
    return 0
def _wait_mouseup_async(device,callback,args,params):
    ret=False
    while not ret:
        up=device.wait_mouseup(*params)
        ret=callback(up,*args)
    return 0
def on_mouseup(device,callback,args=(),params=(1,0,0)):
    threading.Thread(target=_wait_mouseup_async,args=(callback,params)).start()
    return 0
def _wait_scroll_async(device,callback,args):
    ret=False
    while not ret:
        up=device.wait_scroll()
        ret=callback(up,*args)
    return 0
def on_scroll(device,callback,args=()):
    threading.Thread(target=_wait_mouseup_async,args=(device,callback,args)).start()
    return 0