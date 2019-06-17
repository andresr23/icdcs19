CORE=${1:-0}
rm spectre.dat -f
echo 2 > /sys/devices/cpu/rdpmc
wait
cd ../drivers/amd
taskset -c $CORE insmod pmc.ko
wait
cd ../../spectre
taskset -c $CORE ./spectre
wait
taskset -c $CORE rmmod pmc
echo 1 > /sys/devices/cpu/rdpmc
wait
#rm spectre.png -f
#gnuplot spectre.gnuplot
