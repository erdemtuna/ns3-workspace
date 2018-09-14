#!/usr/bin/env gnuplot

set terminal pngcairo solid enhanced font "arial,12" fontscale 1.0 size 1440, 720
set out '9enb-1ue-sinr-plot-with-cell-ids-dluplink.png'
set key fixed left top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set title "9 LTE eNBs and 1 UE Scenario "
set title  font ",20" norotate

#set sample count for smoothing operation
set samples 700

#set the line style
set style line 1 lt 2 lc rgb "red" lw 3
set style line 2 lt 2 lc rgb "orange" lw 3

#x-axis properties
set xrange [0:90]
set xlabel "Time (s)"

#y1-axis properties
set ytics 5 nomirror tc rgb "red"
set ylabel 'SINR (dB)'
set yrange [20:70]

#y2-axis properties
set y2tics 1 nomirror tc rgb "blue"
set y2label 'Connected LTE eNB ID' 
set y2range [0:10]

#set the grids
set grid ytics lc rgb "#393939" lw 1 lt 0
set grid xtics lc rgb "#393939" lw 1 lt 0

plot "DlRsrpSinrStats.txt" u 1:(10*log10($6)) smooth mcsplines ls 1 lw 2 title ' Downlink SINR' axes x1y1, \
     "UlSinrStats.txt" u 1:(10*log10($5)) smooth mcsplines ls 2 lw 2 title ' Uplink SINR' axes x1y1, \
     "UlSinrStats.txt" u 1:2 smooth mcsplines ls 1 lw 2 lc rgb "blue" title ' ID' axes x1y2