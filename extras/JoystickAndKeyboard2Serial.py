import sys
import serial
import time
# import os

# os.environ["SDL_VIDEODRIVER"] = "dummy"

import pygame as pg

pg.init()
screen = pg.display.set_mode((1, 1))
clock = pg.time.Clock()

refresh_rate = 20
baudrate = 115200
port = 6;


# init joysticks and get joystick
joystick = None
joystick_type = "Controller (XBOX 360 For Windows)"

pg.joystick.init()
# get joysticks
joysticks = [pg.joystick.Joystick(x) for x in range(pg.joystick.get_count())]
# select xBox controller
print("devices:")
for j in joysticks:
    name = j.get_name()
    print(name)
    if name == joystick_type:
        joystick = j
        
if joystick == None:
    print("could not detect: ", joystick_type, "\n")
    print("please connect device and restart program\n")
else:
    joystick.init()
    print("Joystick: " + joystick.get_name())

ser = None

while ser == None:
    # if port is chosen
    if port:
        try:
            ser = serial.Serial('COM' + str(port), baudrate, timeout = 1 / refresh_rate)
        except:
            pass
    else:
        # try all ports:
        for i in range(10):
            try:
                ser = serial.Serial('COM' + str(i), baudrate)
                print("Port = COM" + str(i))
                break
            except:
                pass
                
    if ser == None:
        print("Waiting for serial device")
        time.sleep(1)

print("found serial")
        
buttons = {
    0: "A",
    1: "B",
    2: "X",
    3: "Y",
}


hat_to_button = {
    (1, 0): "B",
    (0, 1): "Y",
    (-1, 0): "X",
    (0, -1): "A"
}


def send_serial(button):
    ser.write((button.encode()))


def my_quit():
    global loop
    loop = False


def key_up():
    send_serial("w")
    
 
def key_down():
    send_serial("s")
    
    
def key_left():
    send_serial("a")
    

def key_right():
    send_serial("d")
    

keydown_func = {
    pg.K_ESCAPE: my_quit,
    pg.K_RIGHT: key_right,
    pg.K_LEFT: key_left,
    pg.K_UP: key_up,
    pg.K_DOWN: key_down,
}

loop = True
while loop:
    clock.tick(refresh_rate)
    for e in pg.event.get():
        if e.type == pg.QUIT:
            loop = False
        elif e.type == pg.KEYDOWN:
            try:
                keydown_func[e.key]()
            except:
                sys.stderr.write("No such Key!")
                sys.stderr.flush()
        elif e.type == pg.JOYBUTTONDOWN:
            if e.button in buttons:
                send_serial(buttons[e.button])
        elif e.type == pg.JOYHATMOTION:
            if e.value in hat_to_button:
                send_serial(hat_to_button[e.value])
    
    print_str = str(ser.readline().decode().strip())
    
    if print_str:
        print(print_str)
    
pg.quit()
ser.close()
