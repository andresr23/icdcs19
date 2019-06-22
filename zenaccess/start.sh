CORE=${1:-0}
rm zenaccess.dat -f
echo 2 > /sys/devices/cpu/rdpmc
wait
cd ../drivers/amd
taskset -c $CORE insmod pmc.ko
wait
cd ../../zenaccess
taskset -c $CORE ./zenaccess 7
wait
taskset -c $CORE rmmod pmc
echo 1 > /sys/devices/cpu/rdpmc
wait
rm zenaccess.png -f
gnuplot zenaccess.gnuplot
