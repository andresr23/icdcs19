set terminal png font 'Helvetica, 15' size 640, 400
set output 'zenaccess.png'
set xrange [-1:16]
set xtics 0,1,15
set xlabel "L2 Sector"
set ylabel "Delta_{PMC}"
set multiplot layout 2,1
set yrange [-0.1:1.1]
set ytics 0,1,1
plot "zenaccess.dat" u:1 with linespoints lc "black" title "0x76"
set yrange [90:120]
set ytics (96,105)
plot "zenaccess.dat" u:2 with linespoints lc "black" title "0x60"
unset multiplot 
