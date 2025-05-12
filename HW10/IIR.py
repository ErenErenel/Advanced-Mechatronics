import numpy as np
import matplotlib.pyplot as plt

# Function to read CSV file and extract time and signal
def read_csv(filename):
    data = np.loadtxt(filename, delimiter=',')
    time = data[:, 0]  # Time values
    signal = data[:, 1]  # Signal values
    return time, signal

# IIR Filter function
def iir_filter(signal, A, B):
    filtered_signal = np.zeros_like(signal)
    filtered_signal[0] = signal[0]  # Initialize the first value
    for i in range(1, len(signal)):
        filtered_signal[i] = A * filtered_signal[i - 1] + B * signal[i]
    return filtered_signal

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

# Plotting function
def plot_filtered_signal(time, signal, filtered_signal, frq, fft_raw, fft_filtered, filename, A, B):
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

    # Plot the time-domain signals
    ax1.plot(time, signal, 'k', label='Unfiltered')  # Original in black
    ax1.plot(time, filtered_signal, 'r', label=f'Filtered (IIR, A={A}, B={B})')  # Filtered in red
    ax1.set_title(f'Time-Domain Signal: {filename} (IIR A={A}, B={B})')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.legend()
    ax1.grid(True)

    # Plot the frequency-domain signals (FFT)
    ax2.loglog(frq, fft_raw, 'k', label='Unfiltered FFT')
    ax2.loglog(frq[:len(fft_filtered)], fft_filtered, 'r', label=f'Filtered FFT (A={A}, B={B})')
    ax2.set_title(f'Frequency-Domain Spectrum (FFT): {filename} (IIR A={A}, B={B})')
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.legend()
    ax2.grid(True, which='both', linestyle='--', linewidth=0.5)

    plt.tight_layout()
    plt.show()

# List of CSV files to process
csv_files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']
# Different weights for IIR filter (A + B = 1)
weights = [(0.90, 0.1)]

# Loop through each file and process
for filename in csv_files:
    time, signal = read_csv(filename)
    sample_rate = len(time) / (time[-1] - time[0])  # Sampling rate

    # Calculate FFT of the raw signal
    frq, fft_raw = calculate_fft(signal, sample_rate)

    # Try different A and B combinations for IIR
    for A, B in weights:
        # Apply IIR Filter
        filtered_signal = iir_filter(signal, A, B)

        # Calculate FFT of the filtered signal
        frq_filtered, fft_filtered = calculate_fft(filtered_signal, sample_rate)

        # Plot both time-domain and frequency-domain signals
        plot_filtered_signal(time, signal, filtered_signal, frq, fft_raw, fft_filtered, filename, A, B)
