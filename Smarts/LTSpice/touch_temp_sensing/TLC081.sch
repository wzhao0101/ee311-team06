*version 8.0 2173821775
lib "D:\SPICE\Burr Brn\TLC081\TLC081.slb" 1047929220
u 447
V? 1
U? 2
R? 1
C? 1
? 1
TX? 1
I? 1
IN? 1
PM? 1
@libraries
"D:\SPICE\Burr Brn\TLC081\TLC081" [.slb]
@analysis
.AC 1 3 0
+0 101
+1 1k
+2 100meg
.DC 1 0 0 0 1 1
+ 0 0 Vin
+ 0 4 -5
+ 0 5 5
+ 0 6 100m
.TRAN 1 0 0 0
+0 20ns
+1 5ms
.OPT 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
ITL4 100
.STEP 0 0 4
+ 0 cval
+ 4 1pF
+ 5 2pF
+ 6 0.5p
.OP 0 
.INC D:\SPICE\Burr Brn\TLC081\TLC081.mod
@targets
@attributes
@translators
a 0 u 13 0 0 0 hln 100 TANGO=PCB
a 0 u 13 0 0 0 hln 100 SCICARDS=PCB
a 0 u 13 0 0 0 hln 100 PROTEL=PCB
a 0 u 13 0 0 0 hln 100 PCBOARDS=PCB
a 0 u 13 0 0 0 hln 100 PCAD=PCB
a 0 u 13 0 0 0 hln 100 PADS=PCB
a 0 u 13 0 0 0 hln 100 ORCAD=PCB
a 0 u 13 0 0 0 hln 100 EDIF=PCB
a 0 u 13 0 0 0 hln 100 CADSTAR=PCB
a 0 u 13 0 0 0 hln 100 PSPICE=PSPICE
a 0 u 13 0 0 0 hln 100 XILINX=XILINX
a 0 u 13 0 0 0 hln 100 POLARIS=PCB
@setup
unconnectedPins 0
connectViaLabel 0
connectViaLocalLabels 0
NoStim4ExtIFPortsWarnings 1
AutoGenStim4ExtIFPorts 1
@index
pageloc 1 0 3208 
@status
n 0 103:07:26:10:46:46;1061920006 e 
s 0 103:07:26:10:46:47;1061920007 e 
c 103:07:26:10:46:44;1061920004
a 0 103:07:26:10:07:40;1061917660 e 
*page 1 0 1520 970 iA
@ports
port 22 AGND 300 250 h
port 23 AGND 410 220 h
port 26 AGND 300 110 u
port 32 AGND 230 240 h
@parts
part 5 R 280 80 h
a 0 sp 0 0 0 10 hlb 100 PART=R
a 0 s 0 0 0 0 hln 100 PKGTYPE=RC05
a 0 s 0 0 0 0 hln 100 GATE=
a 0 x 0 0 0 0 hln 100 PKGREF=RF
a 0 xp 9 0 3 -2 hln 100 REFDES=RF
a 0 u 13 0 25 -3 hln 100 VALUE=200k
part 328 VPULSE 230 200 h
a 1 xp 9 0 30 22 hcn 100 REFDES=Vin
a 1 u 0 0 0 0 hcn 100 DC=0
a 1 u 0 0 0 0 hcn 100 TR=5n
a 1 u 0 0 0 0 hcn 100 TF=5n
a 1 u 0 0 0 0 hcn 100 I1=1pA
a 1 u 0 0 0 0 hcn 100 I2=10u
a 1 u 0 0 0 0 hcn 100 AC=10mV
a 1 u 0 0 0 0 hcn 100 V1=-1
a 1 u 0 0 0 0 hcn 100 V2=1
a 1 u 0 0 0 0 hcn 100 TD=0
a 1 u 0 0 0 0 hcn 100 PW=500u
a 1 u 0 0 0 0 hcn 100 PER=1m
a 0 x 0 0 0 0 hln 100 PKGREF=Vin
part 1 VDC 300 150 u
a 0 sp 0 0 22 37 hln 100 PART=VDC
a 0 x 0 0 0 0 hln 100 PKGREF=V+
a 1 xp 9 0 -12 33 hcn 100 REFDES=V+
a 1 u 13 0 -15 22 hcn 100 DC=5V
part 2 VDC 300 250 u
a 0 sp 0 0 22 37 hln 100 PART=VDC
a 0 x 0 0 0 0 hln 100 PKGREF=V-
a 1 xp 9 0 -14 7 hcn 100 REFDES=V-
a 1 u 13 0 -11 18 hcn 100 DC=5V
part 6 R 410 220 v
a 0 sp 0 0 0 10 hlb 100 PART=R
a 0 s 0 0 0 0 hln 100 PKGTYPE=RC05
a 0 s 0 0 0 0 hln 100 GATE=
a 0 x 0 0 0 0 hln 100 PKGREF=RL
a 0 xp 9 0 15 0 hln 100 REFDES=RL
a 0 u 13 0 -1 -1 hln 100 VALUE=10k
part 446 TLC081 260 160 h
a 0 s 11 0 16 32 hln 100 PART=TLC081
a 0 s 0:13 0 0 0 hln 100 PKGTYPE=
a 0 a 0:13 0 0 0 hln 100 PKGREF=U1
a 1 a 9 0 2 -8 hln 100 REFDES=U1
part 411 titleblk 970 720 h
a 1 u 13 0 60 95 hln 100 Revision=- A
a 1 u 13 0 10 10 hln 100 CompanyName=Burr- Brown Corporation
a 1 u 13 0 10 20 hln 100 Line1=POB 11400
a 1 u 13 0 10 30 hln 100 Line2=Tucson, AZ 85706- 1400
a 1 u 13 0 278 20 hln 100 Line3=Neil P. Albaugh
a 1 u 13 0 130 95 hln 100 Date=October 9, 1996
a 1 u 13 0 320 95 hrn 100 PageText=Page      of
a 1 s 13 0 350 10 hcn 100 PAGESIZE=A
a 1 s 13 0 340 95 hrn 100 PAGECOUNT=1
a 1 s 13 0 300 95 hrn 100 PAGENO=1
part 319 nodeMarker 410 180 h
a 0 s 0 0 0 0 hln 100 PROBEVAR=V-:-
a 0 s 0 0 0 0 hln 100 PROBEVAR=
a 0 s 0 0 0 0 hln 100 PROBEVAR=
a 0 s 0 0 0 0 hln 100 PROBEVAR=
a 0 s 0 0 0 0 hln 100 PROBEVAR=
a 0 s 0 0 0 0 hln 100 PROBEVAR=
a 0 s 0 0 0 0 hln 100 PROBEVAR=
a 0 s 0 0 0 0 hln 100 PROBEVAR=
a 0 a 0 0 4 22 hlb 100 LABEL=2
@conn
w 387
a 0 up 0:33 0 0 0 hln 100 V=
s 230 80 230 160 386
a 0 up 33 0 232 120 hlt 100 V=
s 230 80 280 80 437
s 230 160 260 160 439
w 419
a 0 up 0:33 0 0 0 hln 100 V=
s 230 200 260 200 392
a 0 up 33 0 245 199 hct 100 V=
w 375
a 0 up 0:33 0 0 0 hln 100 V=
s 370 80 370 180 380
a 0 up 33 0 372 130 hlt 100 V=
s 320 80 370 80 376
s 410 180 370 180 374
s 350 180 370 180 444
@junction
j 300 110
+ p 1 -
+ s 26
j 300 250
+ p 2 +
+ s 22
j 410 220
+ p 6 1
+ s 23
j 280 80
+ p 5 1
+ w 387
j 410 180
+ p 6 2
+ w 375
j 320 80
+ p 5 2
+ w 375
j 370 180
+ w 375
+ w 375
j 230 240
+ p 328 -
+ s 32
j 230 200
+ p 328 +
+ w 419
j 410 180
+ p 319 pin1
+ p 6 2
j 410 180
+ p 319 pin1
+ w 375
j 300 150
+ p 446 V+
+ p 1 +
j 300 210
+ p 446 V-
+ p 2 -
j 260 160
+ p 446 -
+ w 387
j 260 200
+ p 446 +
+ w 419
j 350 180
+ p 446 OUT
+ w 375
@attributes
a 0 s 0 0 0 0 hln 100 PAGETITLE=
a 0 s 0 0 0 0 hln 100 PAGENO=1
a 0 s 0 0 0 0 hln 100 PAGESIZE=A
a 0 s 0 0 0 0 hln 100 PAGECOUNT=1
@graphics
