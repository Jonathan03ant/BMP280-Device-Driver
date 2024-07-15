import os


DEVICE_FILE = "/dev/utg"

def readFromDevice ():
    try:
        with open(DEVICE_FILE, 'r') as file:
            retrunData = file.read()
            print(f"Read from device: {data}")
            return returnData
    except IOError as e:
        print(f"Error reading from file {e}")
        return None


def writeToDevice(data):
    try:
        with open(DEVICE_FILE, 'w') as file:
            file.write(data)
            print(f"Wrote to device: "{data})
    except IOerror as e:

