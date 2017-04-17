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
LIBS:Sensors_STMicroelectronics
LIBS:Sensors_Bosch
LIBS:environment-sensor-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 5
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ATMEGA32U4RC-A U?
U 1 1 5870F433
P 5575 3850
F 0 "U?" H 4625 5550 50  0000 C CNN
F 1 "ATMEGA32U4RC-A" H 6275 2350 50  0000 C CNN
F 2 "TQFP44" H 5575 3850 50  0001 C CIN
F 3 "" H 6675 4950 50  0000 C CNN
	1    5575 3850
	1    0    0    -1  
$EndComp
$Comp
L Crystal_Small Y?
U 1 1 5872A530
P 4025 2800
F 0 "Y?" H 4025 2900 50  0000 C CNN
F 1 "16MHz" H 4025 2700 50  0000 C CNN
F 2 "Crystals:Crystal_HC49-SD_SMD" H 4025 2800 50  0001 C CNN
F 3 "" H 4025 2800 50  0000 C CNN
	1    4025 2800
	0    1    1    0   
$EndComp
$Comp
L R R?
U 1 1 5872B2FD
P 4250 2800
F 0 "R?" V 4330 2800 50  0000 C CNN
F 1 "R" V 4250 2800 50  0000 C CNN
F 2 "" V 4180 2800 50  0000 C CNN
F 3 "" H 4250 2800 50  0000 C CNN
	1    4250 2800
	1    0    0    -1  
$EndComp
$Comp
L C_Small C?
U 1 1 5872B345
P 3700 2650
F 0 "C?" H 3710 2720 50  0000 L CNN
F 1 "12p" H 3710 2570 50  0000 L CNN
F 2 "" H 3700 2650 50  0000 C CNN
F 3 "" H 3700 2650 50  0000 C CNN
	1    3700 2650
	0    1    1    0   
$EndComp
$Comp
L C_Small C?
U 1 1 5872BDE0
P 3700 2950
F 0 "C?" H 3710 3020 50  0000 L CNN
F 1 "12p" H 3710 2870 50  0000 L CNN
F 2 "" H 3700 2950 50  0000 C CNN
F 3 "" H 3700 2950 50  0000 C CNN
	1    3700 2950
	0    1    1    0   
$EndComp
Text HLabel 3450 2800 0    60   Input ~ 0
GND
Text HLabel 6850 3000 2    60   Output ~ 0
SS_PHY
Text HLabel 8650 2400 2    60   Output ~ 0
SCLK
Text HLabel 3250 2300 0    60   Input ~ 0
RESET
Text HLabel 8650 2600 2    60   Output ~ 0
MISO
Text HLabel 8650 2500 2    60   Output ~ 0
MOSI
$Comp
L R R?
U 1 1 588A2C85
P 3550 2100
F 0 "R?" V 3630 2100 50  0000 C CNN
F 1 "10k" V 3550 2100 50  0000 C CNN
F 2 "" V 3480 2100 50  0000 C CNN
F 3 "" H 3550 2100 50  0000 C CNN
	1    3550 2100
	1    0    0    -1  
$EndComp
Text Label 3900 2300 0    60   ~ 0
~RESET
Text Label 8950 3200 2    60   ~ 0
~RESET
$Comp
L CONN_01X06 P?
U 1 1 588A4606
P 9225 3050
F 0 "P?" H 9225 3400 50  0000 C CNN
F 1 "ISP" V 9325 3050 50  0000 C CNN
F 2 "" H 9225 3050 50  0000 C CNN
F 3 "" H 9225 3050 50  0000 C CNN
	1    9225 3050
	1    0    0    -1  
$EndComp
Text HLabel 8950 3300 0    60   Input ~ 0
GND
$Comp
L CONN_01X10 P?
U 1 1 588A7198
P 9225 5350
F 0 "P?" H 9225 5900 50  0000 C CNN
F 1 "JTAG" V 9325 5350 50  0000 C CNN
F 2 "" H 9225 5350 50  0000 C CNN
F 3 "" H 9225 5350 50  0000 C CNN
	1    9225 5350
	1    0    0    -1  
$EndComp
Text HLabel 8950 5000 0    60   Input ~ 0
GND
Text HLabel 8950 5800 0    60   Input ~ 0
GND
NoConn ~ 9025 5600
NoConn ~ 9025 5500
Text Label 8950 5400 2    60   ~ 0
~RESET
$Comp
L R R?
U 1 1 588A8497
P 6975 4400
F 0 "R?" V 7055 4400 50  0000 C CNN
F 1 "10k" V 6975 4400 50  0000 C CNN
F 2 "" V 6905 4400 50  0000 C CNN
F 3 "" H 6975 4400 50  0000 C CNN
	1    6975 4400
	0    1    1    0   
$EndComp
Text HLabel 7250 4400 2    60   Input ~ 0
GND
Wire Wire Line
	4025 2700 4025 2650
Wire Wire Line
	3800 2650 4025 2650
Wire Wire Line
	4025 2650 4250 2650
Wire Wire Line
	4250 2650 4350 2650
Wire Wire Line
	3800 2950 4025 2950
Wire Wire Line
	4025 2950 4250 2950
Wire Wire Line
	4250 2950 4350 2950
Wire Wire Line
	4025 2950 4025 2900
Connection ~ 4025 2950
Wire Wire Line
	4350 2950 4350 2900
Wire Wire Line
	4350 2900 4425 2900
Wire Wire Line
	4425 2700 4350 2700
Wire Wire Line
	4350 2700 4350 2650
Connection ~ 4250 2650
Connection ~ 4025 2650
Connection ~ 4250 2950
Wire Wire Line
	3525 2950 3600 2950
Wire Wire Line
	3525 2650 3525 2800
Wire Wire Line
	3525 2800 3525 2950
Wire Wire Line
	3525 2650 3600 2650
Wire Wire Line
	3450 2800 3525 2800
Connection ~ 3525 2800
Wire Wire Line
	6675 2400 8450 2400
Wire Wire Line
	8450 2400 8650 2400
Wire Wire Line
	6675 2500 8350 2500
Wire Wire Line
	8350 2500 8650 2500
Wire Wire Line
	6675 2600 8550 2600
Wire Wire Line
	8550 2600 8650 2600
Wire Wire Line
	3250 2300 3550 2300
Wire Wire Line
	3550 2300 4425 2300
Wire Wire Line
	3550 2250 3550 2300
Connection ~ 3550 2300
Wire Wire Line
	3550 1900 3550 1950
Wire Wire Line
	3250 1900 3550 1900
Wire Wire Line
	9025 2800 8550 2800
Wire Wire Line
	8550 2800 8550 2600
Connection ~ 8550 2600
Wire Wire Line
	8950 2900 9025 2900
Wire Wire Line
	8450 2400 8450 3000
Wire Wire Line
	8450 3000 9025 3000
Connection ~ 8450 2400
Wire Wire Line
	8350 2500 8350 3100
Wire Wire Line
	8350 3100 9025 3100
Connection ~ 8350 2500
Wire Wire Line
	8950 3200 9025 3200
Wire Wire Line
	8950 3300 9025 3300
Wire Wire Line
	6675 4900 9025 4900
Wire Wire Line
	6675 5000 8500 5000
Wire Wire Line
	8500 5000 8500 5300
Wire Wire Line
	8500 5300 9025 5300
Wire Wire Line
	9025 5100 6675 5100
Wire Wire Line
	6675 5200 8425 5200
Wire Wire Line
	8425 5200 8425 5700
Wire Wire Line
	8425 5700 9025 5700
Wire Wire Line
	8950 5000 9025 5000
Wire Wire Line
	8950 5800 9025 5800
Wire Wire Line
	8950 5400 9025 5400
Wire Wire Line
	8950 5200 9025 5200
Wire Wire Line
	6825 4400 6675 4400
Wire Wire Line
	7125 4400 7250 4400
Wire Wire Line
	6850 3000 6675 3000
Text HLabel 6850 3600 2    60   Output ~ 0
SDA
Wire Wire Line
	6675 3600 6850 3600
Text Label 3250 1900 2    60   ~ 0
3V3D
Text Label 8950 2900 2    60   ~ 0
3V3D
Text Label 8950 5200 2    60   ~ 0
3V3D
Text Label 3150 1250 0    60   ~ 0
3V3D
Text HLabel 2450 1250 0    60   Input ~ 0
3V3A
$Comp
L L_Small L?
U 1 1 58F6258F
P 2850 1250
F 0 "L?" H 2880 1290 50  0000 L CNN
F 1 "Wurth 74275043" H 2880 1210 50  0000 L CNN
F 2 "" H 2850 1250 50  0000 C CNN
F 3 "" H 2850 1250 50  0000 C CNN
	1    2850 1250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3150 1250 2950 1250
Wire Wire Line
	2750 1250 2450 1250
Text HLabel 8075 4200 2    60   Output ~ 0
P_THRESH
Text HLabel 6850 4100 2    60   Input ~ 0
P_OUT_1
Text HLabel 6850 3900 2    60   Input ~ 0
P_OUT_2
$Comp
L R_Small R?
U 1 1 58F5E5B2
P 7775 4200
F 0 "R?" H 7805 4220 50  0000 L CNN
F 1 "10k" H 7805 4160 50  0000 L CNN
F 2 "" H 7775 4200 50  0000 C CNN
F 3 "" H 7775 4200 50  0000 C CNN
	1    7775 4200
	0    1    1    0   
$EndComp
$Comp
L C_Small C?
U 1 1 58F5E60E
P 7925 4350
F 0 "C?" H 7935 4420 50  0000 L CNN
F 1 "100n" H 7935 4270 50  0000 L CNN
F 2 "" H 7925 4350 50  0000 C CNN
F 3 "" H 7925 4350 50  0000 C CNN
	1    7925 4350
	1    0    0    -1  
$EndComp
Text Notes 7625 4100 0    60   ~ 0
160Hz low pass
Wire Wire Line
	7925 4250 7925 4200
Wire Wire Line
	7875 4200 7925 4200
Wire Wire Line
	7925 4200 8075 4200
Connection ~ 7925 4200
Text HLabel 7875 4500 0    60   Input ~ 0
GND
Wire Wire Line
	7875 4500 7925 4500
Wire Wire Line
	7925 4500 7925 4450
Wire Wire Line
	6675 4100 6850 4100
Wire Wire Line
	6850 3900 6675 3900
Wire Wire Line
	7675 4200 6675 4200
NoConn ~ 6675 4000
Text HLabel 6850 4700 2    60   Input ~ 0
LDR_OUT
Wire Wire Line
	6850 4700 6675 4700
$EndSCHEMATC
