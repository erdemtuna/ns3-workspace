########### GUIDE for 5G+4G+UE Topology ###########
# For 5G eNB use the following line
## set label "mmWave eNB-1" at 60,50 left offset char 1,0 point pointtype 7 pointsize 3 lc rgb "tomato"
#
# For 4G eNB use the following line
## set label "LTE eNB-1" at 60,50 left offset char 1,0 point pointtype 7 pointsize 3 lc rgb "red"
#
# For UE use the following line
## set label "UE-1" at 60,50 left offset char 1,0 point pointtype 7 pointsize 3 lc rgb "blue"
#
# For building use the following line
## set object 1 rect from 98.7643,30.7945 to 116.132,43.4196 front fc "gray"
########### GUIDE for 5G+4G+UE Topology ###########

########### Colors Source ###########
#https://www2.uni-hamburg.de/Wiss/FB/15/Sustainability/schneider/gnuplot/colors.htm

###just a circle example
###set object "9" circle at 50,50 size char 2 fillcolor rgb "red" fillstyle solid

set terminal pngcairo solid enhanced font "arial,12" fontscale 1.0 size 1440, 720
set out 'scenario-7-map.png'
set key fixed left top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set samples 50, 50
set title "Sub-scenario 7"
set title  font ",20" norotate

########### Buildings SECTION ###########
set object 1 rect from 5,20 to 95,130  fc "gray" 
set object 2 rect from 105,20 to 195,130 front fc "gray" 

########### UE SECTION ###########
set label "1" at 0,0 left offset char -2.5,-.5 point pointtype 7 pointsize 3 lc rgb "blue"

set angles degrees

set arrow 1 from 0,0 to 30,0 linewidth 2.000
set label "UE speed (10,0), 10s" at 3,3 front left 

set arrow 2 from 99,0 to 99,45 front linewidth 2.000
set label "UE speed (0,10), 9s" at 97,3 front left rotate by 90

set arrow 4 from 101,90 to 101,45 front linewidth 2.000
set label "UE speed (0,-10), 8s" at 103,88 front left rotate by -90

set arrow 3 from 101,0 to 131,0 front linewidth 2.000
set label "UE speed (10,0), 10s" at 102,3 front left


set label "" at 99,0 front left offset char -3,0 point pointtype 7 pointsize 1 lc rgb "black"
set label "" at 101,0 front left offset char -3,0 point pointtype 7 pointsize 1 lc rgb "black"
set label "" at 101,90 front left offset char -3,0 point pointtype 7 pointsize 1 lc rgb "black"

########### Handovers SECTION ###########

########### eNB SECTION ###########
set label "" at 100,100 front left offset char -2,.5 point pointtype 7 pointsize 3 lc rgb "brown"
set label "1" at 101.5,99.5 front
set label "2" at 200,10 left offset char 1,0 point pointtype 7 pointsize 3 lc rgb "red"
set label "3" at 0,10 left offset char 1,0 point pointtype 7 pointsize 3 lc rgb "red"
set label "4" at 100,100 left offset char -3.5,0.5 point pointtype 7 pointsize 6 lc rgb "red"


#set the x range of the map
set xrange [-10:210]
set mxtics 5

#set the y range of the map
set yrange [-10:140]
set mytics 2


#set the axis labels
set xlabel "X [m]"
set ylabel "Y [m]"
set xlabel  font ",15" norotate
set ylabel  font ",15"

#set the grids
set grid ytics lc rgb "#393939" lw 1 lt 0
set grid xtics lc rgb "#393939" lw 1 lt 0

plot NaN with points pt 5 lc rgb "gray" title "Building", \
	 NaN with points pt 7 lc rgb "brown" title "LTE eNB", \
	 NaN with points pt 7 lc rgb "red" title "  mmWave eNB", \
     NaN with points pt 7 lc rgb "blue" title "UE"
     

set out
