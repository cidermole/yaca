v 20081231 1
C 39200 40800 0 0 0 title-A2.sym
T 55600 41700 9 14 1 0 0 0 1
Yaca heating control
C 48600 42600 1 0 0 ATmega128.sym
{
T 52200 54900 5 10 1 1 0 6 1
refdes=IC1
T 49000 55050 5 10 0 0 0 0 1
device=Atmega128
T 49000 55250 5 10 0 0 0 0 1
footprint=TQFP64_14
T 49000 56600 5 10 0 0 0 0 1
symversion=1.0
}
C 40500 55100 1 90 0 capacitor-1.sym
{
T 39800 55300 5 10 0 0 90 0 1
device=CAPACITOR
T 40000 55300 5 10 1 1 90 0 1
refdes=C1
T 39600 55300 5 10 0 0 90 0 1
symversion=0.1
T 40700 55300 5 10 1 1 90 0 1
value=100 n
T 40500 55100 5 10 0 1 90 0 1
footprint=0805
}
C 41600 55100 1 90 0 capacitor-1.sym
{
T 40900 55300 5 10 0 0 90 0 1
device=CAPACITOR
T 41100 55300 5 10 1 1 90 0 1
refdes=C2
T 40700 55300 5 10 0 0 90 0 1
symversion=0.1
T 41800 55300 5 10 1 1 90 0 1
value=100 n
T 41600 55100 5 10 0 1 90 0 1
footprint=0805
}
C 42600 55100 1 90 0 capacitor-1.sym
{
T 41900 55300 5 10 0 0 90 0 1
device=CAPACITOR
T 42100 55300 5 10 1 1 90 0 1
refdes=C3
T 41700 55300 5 10 0 0 90 0 1
symversion=0.1
T 42800 55300 5 10 1 1 90 0 1
value=100 n
T 42600 55100 5 10 0 1 90 0 1
footprint=0805
}
C 40200 54700 1 0 0 gnd-1.sym
C 41300 54700 1 0 0 gnd-1.sym
C 42300 54700 1 0 0 gnd-1.sym
C 40100 56100 1 0 0 vcc-1.sym
C 41200 56100 1 0 0 vcc-1.sym
C 42200 56100 1 0 0 generic-power.sym
{
T 42400 56350 5 10 1 1 0 3 1
net=AVcc:1
}
N 42400 56100 42400 56000 4
N 41400 56100 41400 56000 4
N 40300 56100 40300 56000 4
N 40300 55100 40300 55000 4
N 41400 55100 41400 55000 4
N 42400 55100 42400 55000 4
C 44400 55900 1 0 0 coil-1.sym
{
T 44600 56300 5 10 0 0 0 0 1
device=COIL
T 44600 56100 5 10 1 1 0 0 1
refdes=L1
T 44600 56500 5 10 0 0 0 0 1
symversion=0.1
T 44400 55900 5 10 0 0 0 0 1
footprint=0805
T 44600 55700 5 10 1 1 0 0 1
value=10 µ
}
C 44000 56000 1 0 0 vcc-1.sym
C 45400 56000 1 0 0 generic-power.sym
{
T 45600 56250 5 10 1 1 0 3 1
net=AVcc:1
}
N 44200 56000 44200 55900 4
N 44200 55900 44400 55900 4
N 45400 55900 45600 55900 4
N 45600 55900 45600 56000 4
N 53100 54400 52500 54400 4
N 53100 54000 52500 54000 4
N 53100 53600 52500 53600 4
T 54200 54300 9 10 1 0 0 0 1
X+
T 54200 54000 9 10 1 0 0 0 1
X-
T 54200 53600 9 10 1 0 0 0 1
Y+
T 52700 54700 9 10 1 0 0 0 1
touchscreen
C 47600 47500 1 0 0 terminal-1.sym
{
T 47910 48250 5 10 0 0 0 0 1
device=terminal
T 47910 48100 5 10 0 0 0 0 1
footprint=smdpad
T 47850 47550 5 10 1 1 0 6 1
refdes=X4
}
N 48700 47600 48500 47600 4
C 54000 54300 1 0 1 terminal-1.sym
{
T 53690 55050 5 10 0 0 0 6 1
device=terminal
T 53690 54900 5 10 0 0 0 6 1
footprint=smdpad
T 53750 54350 5 10 1 1 0 0 1
refdes=X1
}
C 54000 53900 1 0 1 terminal-1.sym
{
T 53690 54650 5 10 0 0 0 6 1
device=terminal
T 53690 54500 5 10 0 0 0 6 1
footprint=smdpad
T 53750 53950 5 10 1 1 0 0 1
refdes=X2
}
C 54000 53500 1 0 1 terminal-1.sym
{
T 53690 54250 5 10 0 0 0 6 1
device=terminal
T 53690 54100 5 10 0 0 0 6 1
footprint=smdpad
T 53750 53550 5 10 1 1 0 0 1
refdes=X3
}
T 47700 47900 9 10 1 0 0 0 1
ISP
C 47600 47100 1 0 0 terminal-1.sym
{
T 47910 47850 5 10 0 0 0 0 1
device=terminal
T 47910 47700 5 10 0 0 0 0 1
footprint=smdpad
T 47850 47150 5 10 1 1 0 6 1
refdes=X5
}
C 46800 53900 1 0 0 terminal-1.sym
{
T 47110 54650 5 10 0 0 0 0 1
device=terminal
T 47110 54500 5 10 0 0 0 0 1
footprint=smdpad
T 47050 53950 5 10 1 1 0 6 1
refdes=X6
}
N 48700 54000 47700 54000 4
N 48500 47200 48700 47200 4
C 47600 43300 1 0 0 terminal-1.sym
{
T 47910 44050 5 10 0 0 0 0 1
device=terminal
T 47910 43900 5 10 0 0 0 0 1
footprint=smdpad
T 47850 43350 5 10 1 1 0 6 1
refdes=X7
}
N 48700 43400 48500 43400 4
C 43600 55100 1 90 0 capacitor-1.sym
{
T 42900 55300 5 10 0 0 90 0 1
device=CAPACITOR
T 43100 55300 5 10 1 1 90 0 1
refdes=C4
T 42700 55300 5 10 0 0 90 0 1
symversion=0.1
T 43800 55300 5 10 1 1 90 0 1
value=100 n
T 43600 55100 5 10 0 1 90 0 1
footprint=0805
}
C 43300 54700 1 0 0 gnd-1.sym
C 43200 56100 1 0 0 generic-power.sym
{
T 43400 56350 5 10 1 1 0 3 1
net=AREF:1
}
N 43400 56100 43400 56000 4
C 48500 44000 1 90 0 generic-power.sym
{
T 47900 44250 5 10 1 1 180 3 1
net=AREF:1
}
N 48700 44200 48500 44200 4
C 40900 51900 1 0 0 connector6-1.sym
{
T 42700 53700 5 10 0 0 0 0 1
device=CONNECTOR_6
T 41000 53900 5 10 1 1 0 0 1
refdes=X8
T 40900 51900 5 10 0 0 0 0 1
footprint=CONNECTOR 6 1
}
N 42600 53300 42800 53300 4
N 42800 53000 42600 53000 4
N 43000 53600 43000 52700 4
N 43000 52700 42600 52700 4
N 42600 52400 44400 52400 4
C 42500 51000 1 0 0 gnd-1.sym
N 42600 52100 42600 51300 4
T 40100 53600 9 10 1 0 0 0 1
CANH
T 40100 53200 9 10 1 0 0 0 1
CANL
T 40100 52900 9 10 1 0 0 0 1
CANL
T 40100 52600 9 10 1 0 0 0 1
CANH
T 40100 52300 9 10 1 0 0 0 1
+VBUS
T 40100 52000 9 10 1 0 0 0 1
-VBUS
C 56100 54100 1 0 1 terminal-1.sym
{
T 55790 54850 5 10 0 0 0 6 1
device=terminal
T 55790 54700 5 10 0 0 0 6 1
footprint=smdpad
T 55850 54150 5 10 1 1 0 0 1
refdes=X9
}
C 56100 53700 1 0 1 terminal-1.sym
{
T 55790 54450 5 10 0 0 0 6 1
device=terminal
T 55790 54300 5 10 0 0 0 6 1
footprint=smdpad
T 55850 53750 5 10 1 1 0 0 1
refdes=X10
}
C 54800 54400 1 0 0 vcc-1.sym
C 54900 53300 1 0 0 gnd-1.sym
N 55200 54200 55000 54200 4
N 55000 54200 55000 54400 4
N 55200 53800 55000 53800 4
N 55000 53800 55000 53600 4
T 45200 47200 9 10 1 0 0 0 1
ISP MISO + serial display
T 46500 47600 9 10 1 0 0 0 1
ISP MOSI
C 44400 51900 1 0 0 lm2576T-1.sym
{
T 44900 53000 5 10 0 0 0 0 1
device=LM2576T
T 44900 53200 5 10 0 0 0 0 1
footprint=PENTAWATT
T 44800 52700 5 10 1 1 0 0 1
refdes=IC2
}
C 46600 51200 1 90 0 diode-1.sym
{
T 46000 51600 5 10 0 0 90 0 1
device=DIODE
T 46100 51500 5 10 1 1 90 0 1
refdes=D1
T 46600 51200 5 10 0 0 90 0 1
footprint=SMT_DIODE 56 38
T 46800 51200 5 10 1 1 90 0 1
value=MBRS130
}
N 46400 52300 46400 52100 4
C 46300 50800 1 0 0 gnd-1.sym
N 46400 51200 46400 51100 4
N 45600 51900 45600 51700 4
N 45600 51700 44800 51700 4
N 44800 51700 44800 51900 4
C 45100 51300 1 0 0 gnd-1.sym
N 45200 51600 45200 51700 4
C 43300 52300 1 90 1 capacitor-4.sym
{
T 42200 52100 5 10 0 0 270 2 1
device=POLARIZED_CAPACITOR
T 42700 51700 5 10 1 1 90 2 1
refdes=C5
T 42600 52100 5 10 0 0 270 2 1
symversion=0.1
T 43300 52300 5 10 0 0 270 2 1
footprint=NICHICON_WT_CAP 75 260 170 680 780 540
T 43400 51400 5 10 1 1 90 2 1
value=47 µ 20 V
}
N 43100 52300 43100 52400 4
C 43000 51000 1 0 0 gnd-1.sym
N 43100 51400 43100 51300 4
C 44200 52300 1 90 1 capacitor-4.sym
{
T 43100 52100 5 10 0 0 270 2 1
device=POLARIZED_CAPACITOR
T 43600 51700 5 10 1 1 90 2 1
refdes=C6
T 43500 52100 5 10 0 0 270 2 1
symversion=0.1
T 44200 52300 5 10 0 0 270 2 1
footprint=NICHICON_WT_CAP 75 260 170 680 780 540
T 44300 51400 5 10 1 1 90 2 1
value=47 µ 20 V
}
N 44000 52300 44000 52400 4
C 43900 51000 1 0 0 gnd-1.sym
N 44000 51400 44000 51300 4
C 46400 50200 1 90 1 capacitor-4.sym
{
T 45300 50000 5 10 0 0 270 2 1
device=POLARIZED_CAPACITOR
T 45800 49600 5 10 1 1 90 2 1
refdes=C7
T 45700 50000 5 10 0 0 270 2 1
symversion=0.1
T 46400 50200 5 10 0 0 270 2 1
footprint=SMT_2PAD_MM100 250 150 290 800 500 0 0
T 46500 49300 5 10 1 1 90 2 1
value=330 µ 5 V
}
C 45500 50200 1 90 1 capacitor-4.sym
{
T 44400 50000 5 10 0 0 270 2 1
device=POLARIZED_CAPACITOR
T 44900 49600 5 10 1 1 90 2 1
refdes=C8
T 44800 50000 5 10 0 0 270 2 1
symversion=0.1
T 45500 50200 5 10 0 0 270 2 1
footprint=SMT_2PAD_MM100 250 150 290 800 500 0 0
T 45600 49300 5 10 1 1 90 2 1
value=330 µ 5 V
}
C 44600 50200 1 90 1 capacitor-4.sym
{
T 43500 50000 5 10 0 0 270 2 1
device=POLARIZED_CAPACITOR
T 44000 49600 5 10 1 1 90 2 1
refdes=C9
T 43900 50000 5 10 0 0 270 2 1
symversion=0.1
T 44600 50200 5 10 0 0 270 2 1
footprint=SMT_2PAD_MM100 250 150 290 800 500 0 0
T 44700 49300 5 10 1 1 90 2 1
value=330 µ 5 V
}
N 46200 50400 46200 50200 4
N 45300 50400 45300 50200 4
N 44400 50400 44400 50200 4
C 44300 48900 1 0 0 gnd-1.sym
C 45200 48900 1 0 0 gnd-1.sym
C 46100 48900 1 0 0 gnd-1.sym
N 46200 49300 46200 49200 4
N 45300 49300 45300 49200 4
N 44400 49300 44400 49200 4
C 46700 52300 1 0 0 coil-1.sym
{
T 46900 52700 5 10 0 0 0 0 1
device=COIL
T 46900 52500 5 10 1 1 0 0 1
refdes=L2
T 46900 52900 5 10 0 0 0 0 1
symversion=0.1
T 46700 52300 5 10 0 0 0 0 1
footprint=SMT_2PAD_MM100 280 300 1030 1345 990 1345 990
T 46900 52100 5 10 1 1 0 0 1
value=100 µ
}
N 47900 50400 47900 52800 4
N 47900 52800 46500 52800 4
N 46500 52800 46500 52500 4
N 46500 52500 46000 52500 4
N 47700 52300 47900 52300 4
N 46000 52300 46700 52300 4
C 44000 50400 1 0 0 vcc-1.sym
N 44200 50400 47900 50400 4
C 59300 53100 1 0 0 PCA82C250-1.sym
{
T 60500 53200 5 10 1 1 0 0 1
device=PCA82C250
T 59700 55100 5 10 1 1 0 0 1
refdes=IC3
T 59700 55500 5 10 0 0 0 0 1
footprint=SO8
}
N 61300 54500 61600 54500 4
N 61600 54500 61600 56100 4
N 46600 56100 61600 56100 4
N 46600 53600 46600 56100 4
N 42600 53600 46600 53600 4
N 42800 53000 42800 53800 4
N 42800 53800 46400 53800 4
N 46400 53800 46400 56300 4
N 46400 56300 61800 56300 4
N 61800 56300 61800 53900 4
N 61800 53900 61300 53900 4
C 60100 55400 1 0 0 vcc-1.sym
N 60300 55400 60300 55300 4
C 59800 52700 1 0 0 gnd-1.sym
N 59900 53100 59900 53000 4
C 60300 52100 1 90 0 resistor-2.sym
{
T 59950 52500 5 10 0 0 90 0 1
device=RESISTOR
T 60000 52300 5 10 1 1 90 0 1
refdes=R1
T 60300 52100 5 10 0 0 90 0 1
footprint=0805
T 60500 52300 5 10 1 1 90 0 1
value=47 k
}
N 60200 53100 60200 53000 4
C 60100 51700 1 0 0 gnd-1.sym
N 60200 52100 60200 52000 4
C 55400 47200 1 0 0 MCP2515.sym
{
T 55800 51500 5 10 1 1 0 0 1
refdes=IC4
T 55800 52200 5 10 0 0 0 0 1
footprint=SO18M
T 55800 52600 5 10 0 0 0 0 1
device=MCP2515
}
N 48600 54000 48600 55200 4
N 48600 55200 56400 55200 4
N 56400 52600 56400 55200 4
N 54400 52600 56400 52600 4
N 54400 52600 54400 48900 4
N 54400 48900 55500 48900 4
N 48700 53600 48400 53600 4
N 48400 53600 48400 55400 4
N 48400 55400 56600 55400 4
N 56600 52400 56600 55400 4
N 56600 52400 54600 52400 4
N 54600 49300 54600 52400 4
N 54600 49300 55500 49300 4
N 55500 49700 54800 49700 4
N 54800 49700 54800 52200 4
N 54800 52200 56800 52200 4
N 56800 52200 56800 55600 4
N 56800 55600 48200 55600 4
N 48200 53200 48200 55600 4
N 48200 53200 48700 53200 4
N 55500 51100 55400 51100 4
N 55400 51100 55400 51400 4
C 55200 51400 1 0 0 vcc-1.sym
N 48700 51000 48100 51000 4
N 48100 51000 48100 53000 4
N 48100 53000 48000 53000 4
N 48000 53000 48000 55800 4
N 48000 55800 57000 55800 4
N 57000 52000 57000 55800 4
N 57000 52000 55000 52000 4
N 55000 52000 55000 50600 4
N 55000 50600 55500 50600 4
N 57600 51100 57900 51100 4
N 57900 51100 57900 54700 4
N 57900 54700 59300 54700 4
N 59300 54400 58100 54400 4
N 58100 54400 58100 50700 4
N 58100 50700 57600 50700 4
C 61400 51500 1 90 0 capacitor-1.sym
{
T 60700 51700 5 10 0 0 90 0 1
device=CAPACITOR
T 60900 51700 5 10 1 1 90 0 1
refdes=C10
T 60500 51700 5 10 0 0 90 0 1
symversion=0.1
T 61600 51700 5 10 1 1 90 0 1
value=100 n
T 61400 51500 5 10 0 1 90 0 1
footprint=0805
}
C 61100 51100 1 0 0 gnd-1.sym
C 61000 52500 1 0 0 vcc-1.sym
N 61200 52500 61200 52400 4
N 61200 51500 61200 51400 4
C 58500 49100 1 90 0 capacitor-1.sym
{
T 57800 49300 5 10 0 0 90 0 1
device=CAPACITOR
T 58000 49300 5 10 1 1 90 0 1
refdes=C11
T 57600 49300 5 10 0 0 90 0 1
symversion=0.1
T 58700 49300 5 10 1 1 90 0 1
value=100 n
T 58500 49100 5 10 0 1 90 0 1
footprint=0805
}
C 58200 48700 1 0 0 gnd-1.sym
C 58100 50100 1 0 0 vcc-1.sym
N 58300 50100 58300 50000 4
N 58300 49100 58300 49000 4
N 51300 42700 51300 42400 4
N 51300 42400 53100 42400 4
N 53100 42400 53100 48400 4
N 53100 48400 55500 48400 4
C 58100 46600 1 90 0 capacitor-1.sym
{
T 57400 46800 5 10 0 0 90 0 1
device=CAPACITOR
T 57600 46800 5 10 1 1 90 0 1
refdes=C12
T 57200 46800 5 10 0 0 90 0 1
symversion=0.1
T 58300 46800 5 10 1 1 90 0 1
value=22 p
T 58100 46600 5 10 0 1 90 0 1
footprint=0805
}
C 59100 46600 1 90 0 capacitor-1.sym
{
T 58400 46800 5 10 0 0 90 0 1
device=CAPACITOR
T 58600 46800 5 10 1 1 90 0 1
refdes=C13
T 58200 46800 5 10 0 0 90 0 1
symversion=0.1
T 59300 46800 5 10 1 1 90 0 1
value=22 p
T 59100 46600 5 10 0 1 90 0 1
footprint=0805
}
N 57900 47500 57900 47700 4
N 57600 48100 58900 48100 4
N 58900 48100 58900 47500 4
C 57800 46200 1 0 0 gnd-1.sym
C 58800 46200 1 0 0 gnd-1.sym
N 58900 46600 58900 46500 4
N 57900 46600 57900 46500 4
C 58000 47600 1 0 0 crystal-1.sym
{
T 58200 48100 5 10 0 0 0 0 1
device=CRYSTAL
T 58200 47900 5 10 1 1 0 0 1
refdes=Q1
T 58200 48300 5 10 0 0 0 0 1
symversion=0.1
T 58100 47400 5 10 1 1 0 0 1
value=16 MHz
T 58000 47600 5 10 0 0 0 0 1
footprint=SMT_2PAD_MM100 180 508 966 1873 490 1873 490
}
T 47100 54300 9 10 1 0 0 0 1
ISP
T 47900 43700 9 10 1 0 0 0 1
ISP
N 57600 47700 58000 47700 4
N 58700 47700 58900 47700 4
