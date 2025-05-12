import numpy as np
import matplotlib.pyplot as plt

# Function to read CSV file and extract time and signal
def read_csv(filename):
    data = np.loadtxt(filename, delimiter=',')
    time = data[:, 0]  # Time values
    signal = data[:, 1]  # Signal values
    return time, signal

# FFT calculation function
def calculate_fft(signal, Fs):
    n = len(signal)  # Length of the signal
    T = n / Fs  # Total duration
    k = np.arange(n)
    frq = k / T  # Two-sided frequency range
    frq = frq[range(int(n / 2))]  # One-sided frequency range
    Y = np.fft.fft(signal) / n  # FFT and normalization
    Y = Y[range(int(n / 2))]
    return frq, np.abs(Y)

# Plotting function for FFT
def plot_fft(time, signal, frq, Y, filename):
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

    # Plot the time-domain signal
    ax1.plot(time, signal, 'b', label='Time Domain Signal')
    ax1.set_title(f'Time-Domain Signal: {filename}')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.grid(True)
    ax1.legend()

    # Plot the frequency-domain (FFT) spectrum
    ax2.loglog(frq, Y, 'r', label='FFT Spectrum')
    ax2.set_title(f'Frequency-Domain Spectrum (FFT): {filename}')
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.grid(True, which='both', linestyle='--', linewidth=0.5)
    ax2.legend()

    plt.tight_layout()
    plt.show()

# List of CSV files to process
csv_files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

# Process each file and plot FFT
for filename in csv_files:
    time, signal = read_csv(filename)
    sample_rate = len(time) / (time[-1] - time[0])  # Sampling rate

    # Calculate FFT
    frq, Y = calculate_fft(signal, sample_rate)

    # Plot FFT and time-domain signal
    plot_fft(time, signal, frq, Y, filename)
