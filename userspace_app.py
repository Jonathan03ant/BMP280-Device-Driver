import os

DEVICE_FILE = "/dev/utg"

def readFromDevice():
    try:
        with open(DEVICE_FILE, 'r') as file:
            returnData = file.read()  
            print(f"Read from device: {returnData}") 
            return returnData
    except IOError as e:
        print(f"Error reading from file: {e}")
        return None

def writeToDevice(data):
    try:
        with open(DEVICE_FILE, 'w') as file:
            file.write(data)
            print(f"Wrote to device: {data}")  
    except IOError as e:  
        print(f"Error writing to file: {e}")
        return None

def main():  
    # Writing something to device
    writeToDevice("App from User Space writing to utg")

    # Reading from device
    readFromDevice()

if __name__ == "__main__":
    main()
