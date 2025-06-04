# The firmware uses the Kbuild build system. There are 'Kbuild' files in this
# example that outlays what needs to be built. (check src/Kbuild).
#
# The firmware is configured using options in Kconfig files, the
# values of these end up in the .config file in the firmware directory.
#
# By setting the OOT_CONFIG (it is '$(PWD)/oot-config' by default) environment
# variable you can provide a custom configuration. It is important that you
# enable the app-layer. See app-config in this directory for example.

#
# We want to execute the main Makefile for the firmware project,
# it will handle the build for us.
#
CRAZYFLIE_BASE := ../crazyflie-firmware

#
# We override the default OOT_CONFIG here, we could also name our config
# to oot-config and that would be the default.
#
OOT_CONFIG := $(PWD)/app-config

include $(CRAZYFLIE_BASE)/tools/make/oot.mk

flash_cfloader:
	cfloader flash build/cf2.bin stm32-fw

init_dongle:
	@sh -c '\
	dev_path=$$(udevadm info /dev/bus/usb/*/* | grep -B50 "Bitcraze Crazyradio" | grep "DEVNAME=" | cut -d"=" -f2); \
	if [ -z "$$dev_path" ]; then \
		echo "No Crazyradio found!"; \
		exit 1; \
	else \
		echo "Found Crazyradio at $$dev_path"; \
		sudo chmod 777 $$dev_path; \
	fi'

init_usb:
	@sh -c '\
	dev_path=$$(udevadm info /dev/bus/usb/*/* | grep -B50 "Bitcraze_AB_Crazyflie" | grep "DEVNAME=" | cut -d"=" -f2); \
	if [ -z "$$dev_path" ]; then \
		echo "No Crazyflie found!"; \
		exit 1; \
	else \
		echo "Found Crazyflie at $$dev_path"; \
		sudo chmod 777 $$dev_path; \
	fi'
