Use ADB (Android Debug Bridge) to do production test report reading
- Power on device.
- Connect device to host via USB.
- Open command prompt on host and go to directory where adb and
	synaptics_production_test reside.
- Run "adb devices" to ensure connection with device.
- Run "adb root" to have root privileges.
- Run "adb push synaptics_production_test /data" to copy
	synaptics_production_test to /data directory on device.
- Run "adb shell chmod 777 /data/synaptics_production_test" to make
	synaptics_production_test executable.
- Run "adb shell /data/synaptics_production_test 20" to read production test
	report type 20.

Pleae make sure CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_TEST_REPORTING is enabled in
the defconfig file when compiling the driver.

Parameters
[-d {sysfs_entry}]	- Path to sysfs entry of sensor
                     (default path is /sys/class/input/input0)
[-s]	- Skip preparation procedures
[-i] 	- Show all test results
[-r] 	- Run 4 tests
[-x {file_name.xls}]	- Load test limits from an external .xls file