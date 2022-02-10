import sys
import serial
import time
from pynput import keyboard

refresh_rate = 20
baudrate = 115200
port = 6;

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

print("Found serial")
print("Terminate with esc")
           
def send_serial(button):
    ser.write((button.encode()))
    
def on_press(key):
    if key == keyboard.Key.esc:
        ser.close()
        sys.exit()  
        return False  # stop listener
    try:
        k = key.char  # single-char keys
    except:
        k = key.name  # other keys
    if k in [ 'w', 'a', 's', 'd']:  # keys of interest
        send_serial(k)
#        return False  # stop listener; remove this if want more keys

listener = keyboard.Listener(on_press=on_press)
listener.start()  # start to listen on a separate thread
listener.join()  # remove if main thread is polling self.keys
