set terminal png font 'Helvetica, 15' size 960, 640
set key top left
set output 'llc.png'
set xrange [-1:33]
set xtics 0,1,32
set xlabel "# of cache lines accessed from U"
set ylabel "Delta"
set yrange [50:460]
set ytics 50,50,450
set multiplot layout 2,1
plot "llc.dat" u:1 with linespoints lc "black" title "TSC"
plot "llc.dat" u:2 with linespoints lc "black" title "PMC 0x076"
unset multiplot 
