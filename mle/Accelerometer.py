import numpy as np
import pandas as pd
import os
import matplotlib.pyplot as plt
g = 9.81
M = 9

class PositionEstimator:
    def __init__(self):
        self.raw_calibration_data = np.array([])
        self.calibration_bias = np.array([0.0, 0.0, 0.0]) # x, y, z
        self.calibration_misalignment = np.array([0.0, 0.0, 0.0]) # yz,zy,zx
        self.calibration_scale = np.array([1.0, 1.0, 1.0]) # x, y, z
        self.acceleration = np.array([0.0, 0.0, 0.0])
        self.velocity = np.array([0.0, 0.0, 0.0])
        self.position = np.array([0.0, 0.0, 0.0])
        self.count = 0
        self.N = 480 # Number of samples for calibration
    
    def load_data(self,file_path):
        df = pd.read_csv(file_path)
        accel_data = df.iloc[:, 1:4].values * g # convert to m/s^2
        return accel_data
    
    def load_all_data(self,folder_path):
        accel_list= []
        for file_name in os.listdir(folder_path):
            if file_name.endswith(".csv"):
                file_path = os.path.join(folder_path, file_name)
                accel_data = self.load_data(file_path)
                accel_data= accel_data[:self.N]
                accel_list.append(accel_data)
        accel_data = np.vstack(accel_list)
        invalid_indices =  np.where(np.isnan(accel_data))
        accel_data = np.delete(accel_data, invalid_indices, axis=0)
        return accel_data
    
    def calibrate(self,data_path):
        """Calculate the zero reference point based on initial samples (assume no movement at start)."""
        self.raw_calibration_data = self.load_all_data(data_path)
        # to be implemented
        self.calibration_bias = np.mean(self.raw_calibration_data, axis=0)
        self.calibration_scale = np.std(self.raw_calibration_data, axis=0)
        self.calibration_misalignment = np.std(self.raw_calibration_data, axis=0)
    
    def apply_calibration(self,data_path):
        raw_data = self.load_all_data(data_path)
        T_inv = np.array([
            [1, self.calibration_misalignment[0], -self.calibration_misalignment[1]],
            [0, 1, self.calibration_misalignment[2]],
            [0, 0, 1]
        ])
        
        # Compute the correction matrix T = inverse(T_inv)
        T = np.linalg.inv(T_inv)
        
        # Construct scale factor matrix K and its inverse
        K = np.diag([self.calibration_scale[0], self.calibration_scale[1], self.calibration_scale[2]])
        K_inv = np.diag(1 / np.array([self.calibration_scale[0],self.calibration_scale[1], self.calibration_scale[2]]))
        
        # Bias vector
        b = np.array([self.calibration_bias[0], self.calibration_bias[1], self.calibration_bias[2]])
        
        # Apply calibration to each sample
        calibrated_data = np.zeros_like(raw_data)
        for i in range(raw_data.shape[0]):
            calibrated_data[i] = T @ K_inv @ (raw_data[i] - b)
        
        return calibrated_data
    
    def filter_data(self,data):
        """Apply a simple moving average filter to smooth the acceleration data."""
        window_size = 8
        filtered_data = np.array([
            np.convolve(data[:, i], np.ones(window_size)/window_size, mode='valid') 
            for i in range(3)
        ]).T  # Transpose to maintain shape (N, 3)
        return filtered_data

    def compute_position(self,data,output_file):
        """Perform double integration to estimate position in 3D space."""
        filtered_data = self.filter_data(data)
        for acceleration in filtered_data:
            # Apply threshold to remove small variations (noise filtering)
            acceleration[np.abs(acceleration) <= 0.3] = 0  # Eliminating small noise values
            new_velocity = self.velocity + self.acceleration + (acceleration - self.acceleration) / 2 
            new_position = self.position + self.velocity + (new_velocity - self.velocity) / 2

            # Update state
            self.acceleration = acceleration
            self.velocity = new_velocity
            self.position = new_position

            # Movement end detection (if acceleration is consistently zero)
            if np.all(acceleration == 0):
                self.count += 1
            else:
                self.count = 0

            if self.count >= 25:
                self.velocity[:] = 0  # Stop velocity if no movement detected
                self.count = 0
        
        # Save position data to file
        df = pd.DataFrame([self.position], columns=['X', 'Y', 'Z'])
        df.to_csv(output_file, index=False)
        return self.position

# Example usage
processor = PositionEstimator()
processor.calibrate('data/calibration')
calibrated_data = processor.apply_calibration('movement')
calibrated_pos = processor.compute_position(calibrated_data, 'calibrated_position.csv')
raw_position = processor.load_all_data('movement')
raw_pos = processor.compute_position(raw_position, 'raw_position.csv')

# Plot raw position vs calibrated position
plt.figure(figsize=(12, 6))

# Raw position
plt.subplot(1, 2, 1)
plt.plot(raw_pos[:, 0], label='X')
plt.plot(raw_pos[:, 1], label='Y')
plt.plot(raw_pos[:, 2], label='Z')
plt.title('Raw Position')
plt.xlabel('Sample')
plt.ylabel('Position (m)')
plt.legend()

# Calibrated position
plt.subplot(1, 2, 2)
plt.plot(calibrated_pos[:, 0], label='X')
plt.plot(calibrated_pos[:, 1], label='Y')
plt.plot(calibrated_pos[:, 2], label='Z')
plt.title('Calibrated Position')
plt.xlabel('Sample')
plt.ylabel('Position (m)')
plt.legend()

plt.tight_layout()
plt.show()
