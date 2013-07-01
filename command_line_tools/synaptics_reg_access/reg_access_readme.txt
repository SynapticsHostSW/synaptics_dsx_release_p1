Use ADB (Android Debug Bridge) to do register access
- Power on device.
- Connect device to host via USB.
- Open command prompt on host and go to directory where adb and reg_access
  reside.
- Run "adb devices" to ensure connection with device.
- Run "adb root" to have root privileges.
- Run "adb push reg_access /data" to copy reg_access to /data directory on
  device.
- Run "adb shell chmod 777 /data/reg_access" to make reg_access executable.
- Follow instructions below to run reg_access.

Parameters
[-a {address in hex}] - Start address (16-bit) for reading/writing
[-l {length to read}] - Length in bytes to read from start address
[-d {data to write}] - Data (MSB = first byte to write) to write to start address
[-r] - Read from start address for number of bytes set with -l parameter
[-w] - Write data set with -d parameter to start address

Usage examples
- Read five bytes of data starting from address 0x048a
   adb shell /data/reg_access -a 0x048a -l 5 -r
- Write 0x11 0x22 0x33 to address 0x048a starting with 0x11
   adb shell /data/reg_access -a 0x048a -d 0x112233 -w
