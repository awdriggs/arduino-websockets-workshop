PORT := /dev/cu.usbmodem1101
FQBN := arduino:samd:nano_33_iot
SKETCH := .

compile:
	arduino-cli compile --fqbn $(FQBN) $(SKETCH)

upload: compile
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(SKETCH)

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=9600

.PHONY: compile upload monitor
