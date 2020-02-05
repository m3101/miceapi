import mmapi
mouse=mmapi.Device('/dev/input/event5')
print("Please click to continue...")
print(str(mouse.wait_mousedown(0,0,1)))
print("\nThanks!")