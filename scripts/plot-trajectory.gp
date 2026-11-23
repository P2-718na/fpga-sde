# Gnuplot script to plot neuron trajectories
# Input: net-accel/build/trajectory.csv

set datafile separator ","
set key autotitle columnhead outside top right
set xlabel "Time (s)"
set ylabel "u"
set grid

datafile = "net-accel/build/trajectory.csv"

# Plot u trajectories: Hardware (solid) vs CPU Reference (dashed)
plot datafile using 1:2 with lines lw 2 title "Neuron 0 u (HW)", \
     datafile using 1:4 with lines dt 2 lw 2 title "Neuron 0 u (Ref)", \
     datafile using 1:6 with lines lw 2 title "Neuron 1 u (HW)", \
     datafile using 1:8 with lines dt 2 lw 2 title "Neuron 1 u (Ref)"
