import numpy as np
import mouse

import readSerial

if __name__ == '__main__':
    # get serial object
    ser = readSerial.serial_init()
    # focused mode: cursor moves at 25% speed
    focused = False 
    while True: 
        # data1: middle finger
        # data2: ring finger
        # data3: thumb
        # data4: index finger
        data1, data2, data3, data4 = readSerial.get(ser)
        if np.sum(data4) == 0:
            print("Potential Error!")
        # middle and ring fingers are curled: change of gravity direction
        if data1[0, 5] < -4 and data2[0, 5] < -4:
            amp = 0.5
            focused = True
            print("focused")
        # normal mode
        elif data1[0, 5] > -2 and data2[0, 5] > -2:
            amp = 2
            if focused:
                mouse.click()
                print("clicked")
            focused = False
        mouse.move(data4[0, 3] * amp, data4[0, 4] * amp, absolute=False)