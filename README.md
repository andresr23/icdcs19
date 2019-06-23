# Cache Event Detection in AMD Ryzen Processors

This project is intended to demonstrate the functionality (and limitations) of utilizing Performance Monitor Counters to infer cache events within the fabric of AMD Ryzen processors. The repository includes three demonstrative programs to do so and two drivers to configure PMCs on selected physical threads (or logical cores).

## A Quick Demonstration of Spectre

This program provides a visualization of the Spectre vulnerability that was recently found to affect AMD processors. While this program does not violate any security guarantees, it serves to demonstrate how PMCs can be utilized to infer the effects on the cache when spying on the value of a changing "secret" variable located within the program's own address space.

## ZenAccess

This demonstration provides evidence on how AMD processors can predict memory accesses with strides of 4-kB, which are often considered to be unpredictable in Intel processors. When constructing eviction sets to target the L2 cache of a processor's physical core, there is a chance that PMC's won't identify valid eviction sets as L2 misses will change to L1D hits. Additionally, recent attacks such as Meltdown, Fallout, and ZombieLoad (RIDL) use a series of memory accesses (all with strides of 4-kB) to infer micro-architectural changes within the cache via Flush+Reload. Thus alternative detection methodologies should be utilized within AMD Ryzen processors.

## Last Level Cache (LLC) Eviction

This program makes use of PMCs to showcase that AMD Ryzen processors do not implement a slicing mechanism for their LLCs. Additionally, this program demonstrates that the replacement policy of the implemented LLC is pseudo-LRU as consecutive accesses executed by a single core to congruent cache lines will quickly cause an eviction. The number of required accesses corresponds to the sum of the associativity of the L2 cache and the LLC.

## Build the Projects

Each subdirectory contains an individual Makefile. All three projects can be built directly from the root directory via:

```bash
make
```

## Build the Driver

To build the driver for AMD, in the root directory run:

```bash
make driver
```

## Test

To test each project, move to the respective directory and run:

```bash
sudo ./start.sh
```

The start.sh script might need to be changed to executable, which can be done by running:
```bash
sudo chmod +x start.sh
```

Each directory includes a simple gnuplot script to visualize the results quickly.

## Disclaimer

The driver should only be used for academic purposes under your own risk. Be careful when modifying the pmc.c file as altering the value of the control registers can easily cause the computer to completely black-screen. After using the driver, in some cases after boot the OS will give an alert that a problem was found, this seems to happen more often when installing the driver in computers with AMD processors. Additionally, the configuration MSRs for the PMCs might be reset by programs such as Mozilla Firefox, so it is a good idea to keep it closed during testing.
