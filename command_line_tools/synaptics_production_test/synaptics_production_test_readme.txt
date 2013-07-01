Use ADB (Android Debug Bridge) to do production test
- Power on device.
- Connect device to host via USB.
- Open command prompt on host and go to directory where adb and read_report
  reside.
- Run "adb devices" to ensure connection with device.
- Run "adb root" to have root privileges.
- Run "adb push synaptics_production_test /data" to copy read_report 
	to /data directory on device.
- Run "adb shell chmod 777 /data/synaptics_production_test" to make
	synaptics_production_test executable.
- Run "adb shell /data/synaptics_production_test 20" to read report type 20.
- Report type number in command above can be changed for reading other report
  types.

Parameters
[-d {sysfs_entry}] - Path to sysfs entry of sensor
[-s] - Skip preparation procedures
[-i] - Show detailed test results
[-r] - Run 4 tests with test limit: Fullraw/TxTxShort/RxRxShort/HighResistance
[-x {file_name.xls}] - Load test limits from an external .xls file

Usage examples
- Read report type 3 when sensor is enumerated in /sys/class/input/input0
  	./synaptics_production_test 3

- Read report type 20 when sensor is enumerated in /sys/class/input/input3
  	./synaptics_production_test 20 -d /sys/class/input/input3

- Load test limits from an external .xls (-x TestLimit_file_name.xls) 
	and run 4 main tests:
		./synaptics_production_test -r -x TestLimit_sample.xls

Note
- Default location of sensor enumeration is /sys/class/input/input0
- Preparation procedures (turning off CBC, turning off signal clarity, force
  update, force cal, etc...) are enabled by default except for report types 2
  and 3.
