import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import firwin, lfilter

# Function to read CSV file and extract time and signal
def read_csv(filename):
    data = np.loadtxt(filename, delimiter=',')
    time = data[:, 0]  # Time values
    signal = data[:, 1]  # Signal values
    return time, signal

# FIR Filter Design
def design_fir_filter(cutoff, numtaps, fs, window_type):
    nyquist = fs / 2  # Nyquist frequency
    normalized_cutoff = cutoff / nyquist
    print(f"Designing FIR filter with cutoff={cutoff} Hz, normalized cutoff={normalized_cutoff:.4f}, numtaps={numtaps}")
    fir_coeffs = firwin(numtaps, normalized_cutoff, window=window_type)
    return fir_coeffs

# Apply FIR filter to the signal
def apply_fir_filter(signal, fir_coeffs):
    filtered_signal = lfilter(fir_coeffs, 1.0, signal)
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
def plot_filtered_signal(time, signal, filtered_signal, frq, fft_raw, fft_filtered, filename, cutoff, numtaps, window_type):
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

    # Plot the time-domain signals
    ax1.plot(time, signal, 'k', label='Unfiltered')
    ax1.plot(time[:len(filtered_signal)], filtered_signal, 'r', label=f'Filtered (FIR, Cutoff={cutoff}Hz, Taps={numtaps}, Window={window_type})')
    ax1.set_title(f'Time-Domain Signal: {filename} (FIR, Cutoff={cutoff}Hz, Taps={numtaps}, Window={window_type})')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.legend()
    ax1.grid(True)

    # Plot the frequency-domain signals (FFT)
    ax2.loglog(frq, fft_raw, 'k', label='Unfiltered FFT')
    ax2.loglog(frq[:len(fft_filtered)], fft_filtered, 'r', label=f'Filtered FFT (Cutoff={cutoff}Hz, Taps={numtaps}, Window={window_type})')
    ax2.set_title(f'Frequency-Domain Spectrum (FFT): {filename} (FIR, Cutoff={cutoff}Hz, Taps={numtaps}, Window={window_type})')
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.legend()
    ax2.grid(True, which='both', linestyle='--', linewidth=0.5)

    plt.tight_layout()
    plt.show()


# Best filtering parameters for each signal
best_params = {
    'sigA.csv': {'cutoff': 25, 'window': 'hamming', 'numtaps': 501},
    'sigB.csv': {'cutoff': 3, 'window': 'blackman', 'numtaps': 201},
    'sigC.csv': {'cutoff': 50, 'window': 'hann', 'numtaps': 101},
    'sigD.csv': {'cutoff': 2, 'window': 'blackman', 'numtaps': 31}
}

# Process each file and apply the best FIR filter
csv_files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

for filename in csv_files:
    time, signal = read_csv(filename)
    sample_rate = len(time) / (time[-1] - time[0])  # Sampling rate

    # Get optimal cutoff, window, and taps for the current signal
    cutoff = best_params[filename]['cutoff']
    window_type = best_params[filename]['window']
    numtaps = best_params[filename]['numtaps']

    # Design and apply FIR filter
    fir_coeffs = design_fir_filter(cutoff, numtaps, sample_rate, window_type)
    filtered_signal = apply_fir_filter(signal, fir_coeffs)

    # Calculate FFT for raw and filtered signals
    frq, fft_raw = calculate_fft(signal, sample_rate)
    frq_filtered, fft_filtered = calculate_fft(filtered_signal, sample_rate)

    # Plot the filtered result
    plot_filtered_signal(time, signal, filtered_signal, frq, fft_raw, fft_filtered, filename, cutoff, numtaps, window_type)
