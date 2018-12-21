#! /bin/sh
file1=1-recv-bbr-rate.txt
file2=2-recv-bbr-rate.txt
file3=3-recv-bbr-rate.txt
gnuplot<<!
set xlabel "time/ms" 
set ylabel "bw/kbps"
set xrange [0:350000]
set yrange [0:3200]
set term "png"
set output "recv-bbr.png"
plot "${file1}" u 1:2 title "flow1" with lines lw 2,\
"${file2}" u 1:2 title "flow2" with lines lw 2,\
"${file3}" u 1:2 title "flow3" with lines lw 2
set output
exit
!
