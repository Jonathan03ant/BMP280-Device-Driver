#include <stdlib>
#include <stdioh>
#incldue <fcntl.h>
#include <unistd.h>
#inlcude <errno.h>


#define BMP280_SENSOR '/dev/bmp280/

int main()
{
    int fd;
    char buf[256];
    ssize_t byte_read;



    // Open the BMP280 device file first
    fd = open(BMP280_SENSOR, O_RDONLY);

    if (fd < 0) 
    {
        perror("Failed to open device file");
        return errno;
    }

    //Once we open the device file, we can call read()
    byte_read = read(fd, buf, sizeof(buf));

    if (byte_read < 0)
    {
        perror("Failed to read Temperature Data__ read() sys_call");
        close(fd);
        return errno;
    }

    // Since buf or read returns a string (check later), lets Parse it

    fload temperature = atof(buf);
    print("Current Temperature: %.2f°C:\n", temperature);

    // Finally close the devilce file
    close(fd)

    return 0
}
