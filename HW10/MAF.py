import numpy as np
import matplotlib.pyplot as plt

# Function to read CSV file and extract time and signal
def read_csv(filename):
    data = np.loadtxt(filename, delimiter=',')
    time = data[:, 0]  # Time values
    signal = data[:, 1]  # Signal values
    return time, signal

# Moving Average Filter function with delay adjustment
def moving_average(signal, window_size):
    maf = np.convolve(signal, np.ones(window_size) / window_size, mode='valid')
    # Calculate delay as half the window size
    delay = (window_size - 1) / 2
    return maf, delay

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
def plot_filtered_signal(time, signal, filtered_signal, adjusted_time, frq, fft_raw, fft_filtered, filename, window_size):
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

    # Plot the time-domain signals
    ax1.plot(time, signal, 'k', label='Unfiltered')  # Original in black
    ax1.plot(adjusted_time, filtered_signal, 'r', label=f'Filtered (MAF, X={window_size})')  # Filtered in red
    ax1.set_title(f'Time-Domain Signal: {filename} (MAF X={window_size})')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.legend()
    ax1.grid(True)

    # Plot the frequency-domain signals (FFT)
    ax2.loglog(frq, fft_raw, 'k', label='Unfiltered FFT')
    ax2.loglog(frq[:len(fft_filtered)], fft_filtered, 'r', label=f'Filtered FFT (X={window_size})')
    ax2.set_title(f'Frequency-Domain Spectrum (FFT): {filename} (MAF X={window_size})')
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.legend()
    ax2.grid(True, which='both', linestyle='--', linewidth=0.5)

    plt.tight_layout()
    plt.show()

# List of CSV files to process
csv_files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']
window_sizes = [75]  # Different window sizes for MAF

# Loop through each file and process
for filename in csv_files:
    time, signal = read_csv(filename)
    sample_rate = len(time) / (time[-1] - time[0])  # Sampling rate

    # Calculate FFT of the raw signal
    frq, fft_raw = calculate_fft(signal, sample_rate)

    # Try different window sizes for MAF
    for window_size in window_sizes:
        # Apply Moving Average Filter
        filtered_signal, delay = moving_average(signal, window_size)

        # Adjusted time array for the filtered signal
        adjusted_time = time[:len(filtered_signal)] + delay * (time[1] - time[0])

        # Calculate FFT of the filtered signal
        frq_filtered, fft_filtered = calculate_fft(filtered_signal, sample_rate)

        # Plot both time-domain and frequency-domain signals
        plot_filtered_signal(time, signal, filtered_signal, adjusted_time, frq, fft_raw, fft_filtered, filename, window_size)
