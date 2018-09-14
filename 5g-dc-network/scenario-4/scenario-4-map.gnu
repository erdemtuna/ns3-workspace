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
set out 'scenario-4-map.png'
set key fixed left top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set samples 50, 50
set title "Sub-scenario 4"
set title  font ",20" norotate

########### Buildings SECTION ###########
set object 1 rect from 40,5 to 60,10 front fc "gray" 

########### UE SECTION ###########
set label "1" at 0,15 left offset char -2,-1 point pointtype 7 pointsize 3 lc rgb "blue"

set angles degrees

set arrow 1 from 0,15 to 16,15 linewidth 2.000
set label "UE speed (10,0), 10s" at 2,14.5 left 

########### Handovers SECTION ###########

########### eNB SECTION ###########
set label "" at 50,0 front left offset char -1.5,0 point pointtype 7 pointsize 3 lc rgb "brown"
set label "1" at 50.5,-.5 front

set label "2" at 50,0 left offset char -3.5,0.5 point pointtype 7 pointsize 6 lc rgb "red"


#set the x range of the map
set xrange [-10:100]
set mxtics 5

#set the y range of the map
set yrange [-5:20]
set mytics 2


#set the axis labels
set xlabel "X [m]"
set ylabel "Y [m]"
set xlabel  font ",15" norotate
set ylabel  font ",15"

#set the grids
set grid ytics lc rgb "#393939" lw 1 lt 0
set grid xtics lc rgb "#393939" lw 1 lt 0

plot NaN with points pt 7 lc rgb "brown" title "LTE eNB", \
	 NaN with points pt 7 lc rgb "red" title "  mmWave eNB", \
     NaN with points pt 7 lc rgb "blue" title "UE", \
     NaN with points pt 5 lc rgb "gray" title "Building"

set out
