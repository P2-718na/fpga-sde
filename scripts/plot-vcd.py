import re
import matplotlib.pyplot as plt
import numpy as np

def twos_comp(val, bits):
    """compute the 2's complement of int value val"""
    if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)        # compute negative value
    return val                         # return positive value as is

def parse_vcd(file_path):
    values = []
    signal_map = {}

    with open(file_path, 'r') as f:
        lines = f.readlines()

    # Step 1: Build signal ID → name map
    for line in lines:
        if line.startswith("$var"):
            parts = line.split()
            var_id = parts[3]
            var_name = parts[4]
            signal_map[var_id] = var_name

        if line.startswith("$enddefinitions"):
            break

    # Step 2: Parse value changes
    for line in lines:
        line = line.strip()

        match = re.match(r'b([01xXzZ]+)\s+(\S+)', line)
        if match:
            binary_str = match.group(1)

            if 'x' in binary_str.lower() or 'z' in binary_str.lower():
                continue

            try:
                value = twos_comp(int(binary_str, 2), 32) / (2**24)
                values.append(value)
            except ValueError:
                continue

        match = re.match(r'([01])(\S+)', line)
        if match:
            try:
                values.append(int(match.group(1)))
            except ValueError:
                continue

    return values


def plot_histograms(values):
    nbins = 100
    # First histogram (original)
    counts, bins = np.histogram(values, bins=nbins)

    plt.figure()
    plt.hist(values, bins=nbins)
    plt.title("Histogram of VCD Signal Values")
    plt.xlabel("Value")
    plt.ylabel("Frequency")
    plt.grid()
    plt.show()

    # Second histogram (distribution of bin counts)
    #plt.figure()
    #plt.hist(counts, bins=20)
    #plt.title("Histogram of Bin Counts (Frequency Distribution)")
    #plt.xlabel("Bin Count")
    #plt.ylabel("Number of Bins")
    #plt.grid()
    #plt.show()


if __name__ == "__main__":
    vcd_file = "dump.vcd"
    values = parse_vcd(vcd_file)

    print(f"Parsed {len(values)} values")
    plot_histograms(values)