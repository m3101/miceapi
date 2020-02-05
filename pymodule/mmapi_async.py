#Copyright (c) Amélia O. F. da S.
import threading
def _wait_move_async(device,callback):
    move=device.wait_move()
    callback(move)
    return 0
def wait_move_async(device,callback):
    threading.Thread(target=_wait_move_async,args=(device,callback)).start()
    return 0
def _wait_mousedown_async(device,callback,params):
    down=device.wait_mousedown(*params)
    callback(down)
    return 0
def wait_mousedown_async(device,callback,params=(1,0,0)):
    threading.Thread(target=_wait_mousedown_async,args=(callback,params)).start()
    return 0
def _wait_mouseup_async(device,callback,params):
    up=device.wait_mouseup(*params)
    callback(up)
    return 0
def wait_mouseup_async(device,callback,params=(1,0,0)):
    threading.Thread(target=_wait_mouseup_async,args=(callback,params)).start()
    return 0
def _wait_scroll_async(device,callback):
    up=device.wait_scroll()
    callback(up)
    return 0
def wait_scroll_async(device,callback):
    threading.Thread(target=_wait_mouseup_async,args=(device,callback)).start()
    return 0