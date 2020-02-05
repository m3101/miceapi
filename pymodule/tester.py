import mmapi
import mmapi_async
mouse=mmapi.Device('/dev/input/event5')
print("Please right click to continue...")
mouse.wait_mousedown(0,0,1)
print("Thanks!")
print("Now left click...")
mouse.wait_mousedown()
print("Thanks!")
print("Now left click again...")
def move(dir):
    print("Mouse moved during wait!")
mmapi_async.wait_move_async(mouse,move)
mouse.wait_mousedown()
print("Once more, please.")
mouse.wait_mousedown()