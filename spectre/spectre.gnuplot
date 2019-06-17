set terminal png font 'Helvetica, 15' size 640, 400
set rmargin 1
set lmargin 8
set tmargin 1
set bmargin 3
set xlabel "L1D Cache Set"
set ylabel "Prime+Probe Round"
set cblabel "PMC Increments (Delta)"
set palette grey
set yrange[0:63]
set xrange[0:63]
set cbrange[0:8]
set output 'spectre.png'
plot 'spectre.dat' matrix with image
