v 20100214 2
C 40000 40000 0 0 0 title-B.sym
C 48000 43400 1 0 0 ATmega8.sym
{
T 49700 47600 5 10 1 1 0 6 1
refdes=IC1
T 48300 47800 5 10 0 0 0 0 1
device=ATmega8
T 48000 43400 5 10 0 0 0 0 1
footprint=TQFP32_7
}
T 50200 40900 9 16 1 0 0 0 1
Yaca - MainPower
T 54000 40100 9 10 1 0 0 0 1
David Madl
T 53900 40400 9 10 1 0 0 0 1
2010-09-13
C 47900 48600 1 90 0 capacitor-1.sym
{
T 47200 48800 5 10 0 0 90 0 1
device=CAPACITOR
T 47400 48800 5 10 1 1 90 0 1
refdes=C1
T 47000 48800 5 10 0 0 90 0 1
symversion=0.1
T 48100 48700 5 10 1 1 90 0 1
value=100 n
T 47900 48600 5 10 0 1 0 0 1
footprint=0805
}
C 47600 48300 1 0 0 gnd-1.sym
C 47500 49500 1 0 0 vcc-1.sym
C 48900 48600 1 90 0 capacitor-1.sym
{
T 48200 48800 5 10 0 0 90 0 1
device=CAPACITOR
T 48400 48800 5 10 1 1 90 0 1
refdes=C2
T 48000 48800 5 10 0 0 90 0 1
symversion=0.1
T 49100 48700 5 10 1 1 90 0 1
value=100 n
T 48900 48600 5 10 0 1 0 0 1
footprint=0805
}
C 48600 48300 1 0 0 gnd-1.sym
C 48500 49500 1 0 0 vcc-1.sym
C 52200 42800 1 90 0 capacitor-1.sym
{
T 51500 43000 5 10 0 0 90 0 1
device=CAPACITOR
T 51700 43000 5 10 1 1 90 0 1
refdes=C3
T 51300 43000 5 10 0 0 90 0 1
symversion=0.1
T 52400 42900 5 10 1 1 90 0 1
value=100 n
T 52200 42800 5 10 0 1 0 0 1
footprint=0805
}
C 51900 42500 1 0 0 gnd-1.sym
C 51800 44100 1 0 0 vcc-1.sym
C 51200 42800 1 90 0 capacitor-1.sym
{
T 50500 43000 5 10 0 0 90 0 1
device=CAPACITOR
T 50700 43000 5 10 1 1 90 0 1
refdes=C4
T 50300 43000 5 10 0 0 90 0 1
symversion=0.1
T 51400 42900 5 10 1 1 90 0 1
value=100 n
T 51200 42800 5 10 0 1 0 0 1
footprint=0805
}
C 50900 42500 1 0 0 gnd-1.sym
N 50700 43800 51000 43800 4
N 51000 43800 51000 43700 4
N 52000 44100 52000 43700 4
N 52000 44000 50700 44000 4
C 51000 46600 1 90 0 resistor-2.sym
{
T 50650 47000 5 10 0 0 90 0 1
device=RESISTOR
T 50700 46800 5 10 1 1 90 0 1
refdes=R1
T 51300 46800 5 10 1 1 90 0 1
value=10 k
T 51000 46600 5 10 0 1 90 0 1
footprint=0805
}
C 50700 47500 1 0 0 vcc-1.sym
N 50900 46600 50900 45200 4
N 50700 45200 51100 45200 4
{
T 50700 45200 5 10 0 0 0 0 1
netname=RESET
}
T 41000 48500 9 10 1 0 0 0 1
20 V 2,5 A in
C 45900 40800 1 0 0 header10-2.sym
{
T 45900 42800 5 10 0 1 0 0 1
device=HEADER10
T 46500 42900 5 10 1 1 0 0 1
refdes=X1
T 45900 40800 5 10 0 0 0 0 1
footprint=CONNECTOR 5 2
}
T 45400 42700 9 10 1 0 0 0 1
MOSI
T 47400 42700 9 10 1 0 0 0 1
VTG
T 45600 42300 9 10 1 0 0 0 1
NC
T 47400 42300 9 10 1 0 0 0 1
GND
T 47400 41900 9 10 1 0 0 0 1
GND
T 47400 41500 9 10 1 0 0 0 1
GND
T 47400 41100 9 10 1 0 0 0 1
GND
T 45500 41900 9 10 1 0 0 0 1
RST
T 45500 41500 9 10 1 0 0 0 1
SCK
T 45400 41100 9 10 1 0 0 0 1
MISO
N 47500 44000 48000 44000 4
C 47500 44400 1 90 0 generic-power.sym
{
T 46900 44550 5 10 1 1 0 3 1
net=MOSI:1
}
N 47500 44200 48000 44200 4
N 48000 44400 47500 44400 4
N 47500 44600 47500 44400 4
C 47500 44000 1 90 0 generic-power.sym
{
T 46900 44150 5 10 1 1 0 3 1
net=MISO:1
}
C 47500 43600 1 90 0 generic-power.sym
{
T 46900 43750 5 10 1 1 0 3 1
net=SCK:1
}
N 47500 44000 47500 43800 4
N 45400 41800 45900 41800 4
{
T 45400 41800 5 10 0 0 0 0 1
netname=RESET
}
C 51100 45400 1 270 0 generic-power.sym
{
T 51700 45250 5 10 1 1 180 3 1
net=RESET:1
}
C 45400 42400 1 90 0 generic-power.sym
{
T 44800 42550 5 10 1 1 0 3 1
net=MOSI:1
}
C 45400 40800 1 90 0 generic-power.sym
{
T 44800 40950 5 10 1 1 0 3 1
net=MISO:1
}
C 45400 41600 1 90 0 generic-power.sym
{
T 44800 41750 5 10 1 1 0 3 1
net=RESET:1
}
C 44900 42100 1 0 0 nc-left-1.sym
{
T 44900 42500 5 10 0 0 0 0 1
value=NoConnection
T 44900 42900 5 10 0 0 0 0 1
device=DRC_Directive
}
N 45400 41000 45900 41000 4
N 45400 42200 45900 42200 4
N 45400 42600 45900 42600 4
C 45400 41200 1 90 0 generic-power.sym
{
T 44800 41350 5 10 1 1 0 3 1
net=SCK:1
}
N 45400 41400 45900 41400 4
C 47900 40500 1 0 0 gnd-1.sym
C 47800 42800 1 0 0 vcc-1.sym
N 48000 40800 48000 42200 4
N 48000 42200 47300 42200 4
N 47300 41800 48000 41800 4
N 47300 41400 48000 41400 4
N 47300 41000 48000 41000 4
N 47300 42600 48000 42600 4
N 48000 42600 48000 42800 4
C 40800 48900 1 0 0 connector3-1.sym
{
T 42600 49800 5 10 0 0 0 0 1
device=CONNECTOR_3
T 40800 50000 5 10 1 1 0 0 1
refdes=X2
}
T 41400 49800 9 10 1 0 0 0 1
Pin = 20 V
T 41400 49500 9 10 1 0 0 0 1
Noplug = (GND)
T 41400 49200 9 10 1 0 0 0 1
Shaft = GND
C 44600 49900 1 0 0 generic-power.sym
{
T 44800 50150 5 10 1 1 0 3 1
net=Vin:1
}
C 42800 48600 1 0 0 gnd-2.sym
N 42500 49100 42900 49100 4
N 42900 49100 42900 48900 4
N 42500 49700 43500 49700 4
C 43300 49500 1 180 0 nc-left-1.sym
{
T 43300 49100 5 10 0 0 180 0 1
value=NoConnection
T 43300 48700 5 10 0 0 180 0 1
device=DRC_Directive
}
N 42800 49400 42500 49400 4
C 43500 49600 1 0 0 fuse-2.sym
{
T 43700 50150 5 10 0 0 0 0 1
device=FUSE
T 43700 49900 5 10 1 1 0 0 1
refdes=F1
T 43700 50350 5 10 0 0 0 0 1
symversion=0.1
T 43600 49400 5 10 1 1 0 0 1
value=3 A slow
T 43500 49600 5 10 0 0 0 0 1
footprint=minifuse
}
N 44400 49700 44800 49700 4
N 44800 49700 44800 49900 4
C 42000 46100 1 0 1 IRF1010N-1.sym
{
T 41400 46300 5 10 1 1 0 6 1
device=IRF650A
T 41400 46600 5 10 0 0 0 6 1
footprint=TO-220AB
T 41400 46700 5 10 1 1 0 6 1
refdes=T1
}
C 41600 44900 1 0 1 gnd-2.sym
N 41500 45200 41500 46100 4
C 42100 45300 1 270 1 zener-1.sym
{
T 42700 45700 5 10 0 0 90 2 1
device=ZENER_DIODE
T 42600 45600 5 10 1 1 90 2 1
refdes=D1
T 42000 46000 5 10 1 1 90 6 1
value=15 V
}
C 42400 44900 1 0 1 gnd-2.sym
N 42300 45300 42300 45200 4
N 42000 46300 42300 46300 4
N 42300 46200 42300 46400 4
C 42100 46400 1 270 1 diode-1.sym
{
T 42700 46800 5 10 0 0 90 2 1
device=DIODE
T 42600 46700 5 10 1 1 90 2 1
refdes=D2
T 42000 46500 5 10 1 1 90 0 1
value=1N4148
}
C 42600 46200 1 0 0 resistor-2.sym
{
T 43000 46550 5 10 0 0 0 0 1
device=RESISTOR
T 42800 46500 5 10 1 1 0 0 1
refdes=R2
T 42800 45900 5 10 1 1 0 0 1
value=10 k
T 42600 46200 5 10 0 1 0 0 1
footprint=0805
}
N 42300 46300 42600 46300 4
C 45200 46100 1 0 0 IRF1010N-1.sym
{
T 45800 46300 5 10 1 1 0 0 1
device=IRF650A
T 45800 46600 5 10 0 0 0 0 1
footprint=TO-220AB
T 45800 46700 5 10 1 1 0 0 1
refdes=T2
}
C 45600 44900 1 0 0 gnd-2.sym
N 45700 45200 45700 46100 4
C 45100 45300 1 90 0 zener-1.sym
{
T 44500 45700 5 10 0 0 90 0 1
device=ZENER_DIODE
T 44600 45600 5 10 1 1 90 0 1
refdes=D3
T 45200 46000 5 10 1 1 90 8 1
value=15 V
}
C 44800 44900 1 0 0 gnd-2.sym
N 44900 45300 44900 45200 4
N 45200 46300 44900 46300 4
N 44900 46200 44900 46400 4
C 45100 46400 1 90 0 diode-1.sym
{
T 44500 46800 5 10 0 0 90 0 1
device=DIODE
T 44600 46700 5 10 1 1 90 0 1
refdes=D4
T 45200 46500 5 10 1 1 90 2 1
value=1N4148
}
C 44600 46200 1 0 1 resistor-2.sym
{
T 44200 46550 5 10 0 0 0 6 1
device=RESISTOR
T 44400 46500 5 10 1 1 0 6 1
refdes=R3
T 44400 45900 5 10 1 1 0 6 1
value=10 k
T 44600 46200 5 10 0 1 0 6 1
footprint=0805
}
N 44900 46300 44600 46300 4
N 43700 46300 43500 46300 4
C 43400 46500 1 0 0 generic-power.sym
{
T 43600 46750 5 10 1 1 0 3 1
net=Vin:1
}
N 43600 46500 43600 46300 4