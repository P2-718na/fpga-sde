# Gnuplot script to plot neuron trajectories and network heatmap
# Input: net-accel/build/trajectory.csv

set datafile separator ","
datafile = "net-accel/build/trajectory.csv"

# Automatically extract simulation dimensions from data file
stats datafile skip 1 using 1 nooutput
num_neurons = int(STATS_columns - 1) / 4
t_min = STATS_min
t_max = STATS_max
dt = (STATS_records > 1) ? (t_max - t_min) / (STATS_records - 1) : 0.1

set multiplot layout 2, 1

# -----------------------------------------------------------------------------
# 1. Trajectory Plot: Hardware vs CPU Reference
# -----------------------------------------------------------------------------
set title "Neuron Trajectories (HW vs Ref)"
set key autotitle columnhead outside top right
set xlabel "Time (s)"
set ylabel "u"
set grid

# Plot u trajectories: Hardware (solid) vs CPU Reference (dashed)
plot datafile using 1:2 with lines lw 2 title "Neuron 0 u (HW)", \
     datafile using 1:4 with lines dt 2 lw 2 title "Neuron 0 u (Ref)", \
     datafile using 1:6 with lines lw 2 title "Neuron 1 u (HW)", \
     datafile using 1:8 with lines dt 2 lw 2 title "Neuron 1 u (Ref)"

# -----------------------------------------------------------------------------
# 2. Heatmap Plot: Time on y-axis, Neuron Index on x-axis, Color = neuron_u
# -----------------------------------------------------------------------------
unset grid
unset key
set title "Neuron Activity Heatmap (u)"
set xlabel "Neuron Index"
set ylabel "Time (s)"
set cblabel "u"
set palette viridis

set xrange [-0.5 : num_neurons - 0.5]
set yrange [t_min - dt/2.0 : t_max + dt/2.0]

if (num_neurons <= 20) {
    set xtics 1
} else {
    set xtics auto
}

# boxxyerror: x:y:xdelta:ydelta:color
# Draws a solid filled rectangle for each (neuron, time) point colored by neuron_u
plot for [i=0:num_neurons-1] datafile using (i):1:(0.5):(dt/2.0):(column(2 + 4*i)) \
     with boxxyerror lc palette fillstyle solid notitle

unset multiplot

