EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:wiznet
LIBS:Sensors_Bosch
LIBS:environment-sensor-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 5
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 4800 4550 1250 1550
U 5870F3CD
F0 "Microcontroller" 60
F1 "microcontroller.sch" 60
F2 "SS_PHY" O R 6050 5150 60 
F3 "SCLK" O R 6050 5350 60 
F4 "~RESET" O R 6050 5250 60 
F5 "MISO" O R 6050 5450 60 
F6 "MOSI" O R 6050 5550 60 
F7 "SDA" O R 6050 5050 60 
F8 "3V3A" I R 6050 4650 60 
F9 "GND" I R 6050 4750 60 
F10 "P_THRESH" O R 6050 5825 60 
F11 "P_OUT_1" I R 6050 5925 60 
F12 "P_OUT_2" I R 6050 6025 60 
F13 "LDR_OUT" I R 6050 5725 60 
$EndSheet
$Sheet
S 7300 1000 1175 2650
U 5870F6A9
F0 "Network I/O" 60
F1 "network.sch" 60
F2 "~RESET" I L 7300 3300 60 
F3 "SS_PHY" I L 7300 3200 60 
F4 "MOSI" I L 7300 3600 60 
F5 "MISO" I L 7300 3500 60 
F6 "SCLK" I L 7300 3400 60 
F7 "GND" I L 7300 2650 60 
F8 "3V3A" I L 7300 2550 60 
$EndSheet
$Sheet
S 7300 4100 1175 2000
U 58710409
F0 "Sensors" 60
F1 "sensors.sch" 60
F2 "GND" I L 7300 4375 60 
F3 "SCLK" I L 7300 5350 60 
F4 "SDA" I L 7300 5050 60 
F5 "3V3A" I L 7300 4275 60 
F6 "5VA" I L 7300 4175 60 
F7 "P_OUT_2" O L 7300 6025 60 
F8 "P_OUT_1" O L 7300 5925 60 
F9 "P_THRESH" I L 7300 5825 60 
F10 "LDR_OUT" O L 7300 5725 60 
$EndSheet
$Sheet
S 4800 2375 1250 775 
U 58713E48
F0 "Power Supply" 60
F1 "power.sch" 60
F2 "3V3A" O R 6050 2550 60 
F3 "GND" O R 6050 2650 60 
F4 "5VA" O R 6050 2450 60 
$EndSheet
Wire Wire Line
	6050 5050 7300 5050
Wire Wire Line
	6050 5350 7300 5350
Wire Wire Line
	7300 4275 6250 4275
Wire Wire Line
	6250 2550 6250 4650
Wire Wire Line
	6250 4650 6050 4650
Wire Wire Line
	6350 4750 6050 4750
Wire Wire Line
	6350 2650 6350 4750
Wire Wire Line
	6350 4375 7300 4375
Wire Wire Line
	6050 2550 7300 2550
Connection ~ 6250 4275
Wire Wire Line
	6050 2650 7300 2650
Connection ~ 6350 4375
Connection ~ 6250 2550
Connection ~ 6350 2650
Wire Wire Line
	6050 2450 6150 2450
Wire Wire Line
	6150 2450 6150 4175
Wire Wire Line
	6150 4175 7300 4175
Wire Wire Line
	7300 5825 6050 5825
Wire Wire Line
	6050 5925 7300 5925
Wire Wire Line
	7300 6025 6050 6025
Wire Wire Line
	7300 5725 6050 5725
Wire Wire Line
	6050 5150 6450 5150
Wire Wire Line
	6450 5150 6450 3200
Wire Wire Line
	6450 3200 7300 3200
Wire Wire Line
	6050 5250 6550 5250
Wire Wire Line
	6550 5250 6550 3300
Wire Wire Line
	6550 3300 7300 3300
Wire Wire Line
	7300 3400 6650 3400
Wire Wire Line
	6650 3400 6650 5350
Connection ~ 6650 5350
Wire Wire Line
	7300 3500 6750 3500
Wire Wire Line
	6750 3500 6750 5450
Wire Wire Line
	6750 5450 6050 5450
Wire Wire Line
	7300 3600 6850 3600
Wire Wire Line
	6850 3600 6850 5550
Wire Wire Line
	6850 5550 6050 5550
$EndSCHEMATC
