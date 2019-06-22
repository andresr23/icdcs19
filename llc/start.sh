CORE=${1:-0}
rm llc.dat -f
echo 2 > /sys/devices/cpu/rdpmc
wait
cd ../drivers/amd
taskset -c $CORE insmod pmc.ko
wait
cd ../../llc
taskset -c $CORE ./llc
wait
taskset -c $CORE rmmod pmc
echo 1 > /sys/devices/cpu/rdpmc
wait
#rm llc.png -f
#gnuplot llc.gnuplot
