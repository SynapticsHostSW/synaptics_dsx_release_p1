IMPORTANT NOTE
--------------

This tarball contains alpha-quality pre-release code.  Not all features are
completely implemented.  Not all functions are completely debugged or code
reviewed.  Some features or functions may crash unexpectedly, and others may
not be present at all.

If you find bugs or errors in this code, please let us know.  If you fix
any bugs or errors in this code, or add new enhancements, please send us the
code changes, so we can be sure we implement the changes correctly.


WHAT'S IN THIS TARBALL
----------------------

The following files are provided in this tarball.

drivers/input/touchscreen/synaptics_dsx_xxx.[ch] - the source code for the 
Synaptics DS4 I2C driver.

drivers/input/touchscreen/Makefile - example

drivers/input/touchscreen/Kconfig - example

include/linux/input/synaptics_dsx.h - the RMI4 Linux header file

arch/arm/mach-omap2/board-omap3beagle.c - example board file 
(for Android ICS on BeagleBoard xM)

arch/arm/configs/omap3_beagle_android_defconfig - example defconfig 
(for Android ICS on BeagleBoard xM)



HOW TO INSTALL THIS STUFF
-------------------------

**	Copy drivers/input/touchscreen/synaptics_dsx_xxx.[ch] 
		into the drivers/input/touchscreen directory in your kernel tree.

**	Update Makefile and Kconfig in your kernel tree's drivers/input/touchscreen
		directory for Synaptics touchscreen, using the files in the tarball as an
		example.

**	Copy include/linux/input/synaptics_dsx.h into the 
		include/linux/input/ directory in your kernel tree.

**	Update your board file and properly power on device. Refer to the
   	board-omap3beagle.c file in the tarball as an example.

**	Update your defconfig file.  Refer to omap3_beagle_android_defconfig file in 
		the tarball as an example.	
		"CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_I2C" needs to be defined

**	"make clean" your kernel

**	Make your defconfig

**	Rebuild your kernel

**	Install the new kernel on your Android system.

**	Reboot your Android system.
