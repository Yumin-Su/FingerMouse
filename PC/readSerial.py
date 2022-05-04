import serial, time
import numpy as np

def serial_init():
    """
    Initialize serial object
    """
    ser = serial.Serial('COM10', 614400, timeout=1)
    # discard any potential partial result
    ser.flushInput()
    ser.read_until(b'\x03\x02\x01\x00')
    return ser

def helper(data, offset):
    return np.array([
        int.from_bytes(data[8+offset:10+offset], 'little', signed=True) * 0.0076220,
        int.from_bytes(data[10+offset:12+offset], 'little', signed=True) * 0.0076220,
        int.from_bytes(data[12+offset:14+offset], 'little', signed=True) * 0.0076220,
        int.from_bytes(data[14+offset:16+offset], 'little', signed=True) * 0.000061035 * 9.8,
        int.from_bytes(data[16+offset:18+offset], 'little', signed=True) * 0.000061035 * 9.8,
        int.from_bytes(data[18+offset:20+offset], 'little', signed=True) * 0.000061035 * 9.8,
        int.from_bytes(data[0+offset:2+offset], 'little', signed=True) * 0.3,
        int.from_bytes(data[2+offset:4+offset], 'little', signed=True) * 0.3,
        int.from_bytes(data[4+offset:6+offset], 'little', signed=True) * 0.3
    ], dtype=float).reshape(1,9)

def get(ser):
    data = ser.read_until(b'\x03\x02\x01\x00')
    if len(data) != 4 + 23 * 4:
        print("Serial Error! Retrying...")
        data = ser.read_until(b'\x03\x02\x01\x00')
    return helper(data, 0), helper(data, 23), helper(data, 46), helper(data, 69)

def _test():
    ser = serial.Serial('COM10', 614400, timeout=1)
    ser.flushInput()
    data = ser.read_until(b'\x03\x02\x01\x00')
    f = open("data.csv", "w")
    t1 = []
    t2 = []
    t3 = []
    t4 = []
    t = time.time()
    for _ in range(5000):
        t1.append(time.time())
        data = ser.read_until(b'\x03\x02\x01\x00')
        t2.append(time.time())
        if (len(data) != 23 * 4 + 4):
            print("Error!")
        magn_x = int.from_bytes(data[0:2], 'little', signed=True) * 0.3
        magn_y = int.from_bytes(data[2:4], 'little', signed=True) * 0.3
        magn_z = int.from_bytes(data[4:6], 'little', signed=True) * 0.3
        gyro_x = int.from_bytes(data[8:10], 'little', signed=True) * 0.0076220
        gyro_y = int.from_bytes(data[10:12], 'little', signed=True) * 0.0076220
        gyro_z = int.from_bytes(data[12:14], 'little', signed=True) * 0.0076220
        accel_x = int.from_bytes(data[14:16], 'little', signed=True) * 0.000061035 * 9.8
        accel_y = int.from_bytes(data[16:18], 'little', signed=True) * 0.000061035 * 9.8
        accel_z = int.from_bytes(data[18:20], 'little', signed=True) * 0.000061035 * 9.8
        # print("accel: {},\t{},\t{}\nmagn: {},\t{},\t{}\ngyro: {},\t{},\t{}\n".format(
        #    accel_x, accel_y, accel_z, magn_x, magn_y, magn_z, gyro_x, gyro_y, gyro_z
        # ))
        t3.append(time.time())
        f.write("{},{},{},{},{},{},{},{},{}\n".format(
            gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, magn_x, magn_y, magn_z
        ))
        t4.append(time.time())
    print(time.time() - t)
    f.close()
    t1 = np.array(t1)
    t2 = np.array(t2)
    t3 = np.array(t3)
    print(np.mean(t2-t1))
    print(np.mean(t3-t2))
    print(np.mean(t4-t3))
    print(np.mean(t1[1:]-t4[:-1]))
    print(len(ser.read_all()))

if __name__ == '__main__':
    _test()