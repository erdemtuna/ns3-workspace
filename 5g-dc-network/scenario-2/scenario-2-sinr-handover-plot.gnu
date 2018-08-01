#!/usr/bin/env gnuplot

#set term tikz solid enhanced font "arial,12" fontscale 1.0 size 1440, 720
#set out 'scenario-2-sinr-handover-plot.tex'

set terminal pngcairo solid enhanced font "arial,12" fontscale 1.0 size 1440, 720
set out 'scenario-2-sinr-handover-plot.png'
set key fixed right top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set title "Scenario - 2"
set title  font ",20" norotate

#set sample count for smoothing operation
set samples 700

#set the line style
set style line 1 lt 2 lc rgb "red" lw 3

#x-axis properties
set xrange [0:37]
set xlabel "Time (s)"
set mxtics 5

#y1-axis properties
set ytics 10 nomirror tc rgb "red"
set ylabel 'SINR (dB)'
set yrange [-20:60]
set mytics 2

#y2-axis properties
set y2tics 1 nomirror tc rgb "blue"
set y2label 'Connected eNB ID' 
set y2range [0:4]

#set the grids
set grid ytics lc rgb "#393939" lw 1 lt 0
set grid xtics lc rgb "#393939" lw 1 lt 0

#set -5db threshold
set arrow from graph 0,first -5 to graph 1,first -5 nohead dt 2 lw 2 lc rgb "#000000" front

set multiplot
plot "< awk '$3 == 1' mcMmWaveSwitchStats_5_10_31_07_2018_04_52_12.txt" using ($2):($4) w steps ls 1 lw 1 lc rgb "blue" title ' ID' axes x1y2

set key top left
plot "< awk '$2 == 1' LteSinrTime.txt" using 1:($4) smooth mcsplines ls 1 lw 1 lc rgb "#006400" title ' SINR (eNB 1)' axes x1y1, \
"< awk '$3 == 2' mcMmWaveSinrTime_5_10_31_07_2018_04_52_12.txt" using 1:($4) smooth mcsplines ls 1 lw 1 lc rgb "red" title ' SINR (eNB 2)' axes x1y1, \
"< awk '$3 == 3' mcMmWaveSinrTime_5_10_31_07_2018_04_52_12.txt" using 1:($4) smooth mcsplines ls 1 lw 1 lc rgb "#ff8000" title ' SINR (eNB 3)' axes x1y1
unset multiplot

