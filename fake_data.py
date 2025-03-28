import numpy as np

# Given calibration parameters
A = np.array([[1.060926, 0.012762, -0.030208],
              [0.012762, 0.989113, -0.021786],
              [-0.030208, -0.021786, 1.046190]])
b = np.array([0.094530, -0.011458, 0.005249])

# Function to simulate raw accelerometer data
def generate_raw_accel_data(samples=1000):
    # Simulating random movement around Earth's gravity (±2g range)
    raw_data = np.random.uniform(-2, 2, (samples, 3))
    return raw_data

# Function to add realistic noise
def add_noise(data, noise_level=0.05):
    noise = np.random.normal(0, noise_level, data.shape)
    return data + noise

# Function to apply calibration
def calibrate_accel_data(raw_data):
    return np.array([(A @ raw + b) for raw in raw_data])

# Generate raw accelerometer data
raw_data = generate_raw_accel_data(1000)

# Add noise
noisy_data = add_noise(raw_data, noise_level=0.05)

# Apply calibration
calibrated_data = calibrate_accel_data(noisy_data)

# Save to a text file (tab-separated)
filename = "calibrated_accel_data.txt"
np.savetxt(filename, calibrated_data, delimiter='\t', fmt="%.6f")

print(f"Calibrated accelerometer data saved to {filename}")
