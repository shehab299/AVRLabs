import numpy as np
import pandas as pd
import os
from scipy.optimize import least_squares
import matplotlib.pyplot as plt
g = 9.81
M = 9
max_iterations = 20000
tolerance = 1e-6

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
        self.max_iterations = max_iterations
        self.tolerance = tolerance
    
    def load_data(self,file_path):
        df = pd.read_csv(file_path)
        accel_data = df.iloc[:, 1:4].values*g # convert to m/s^2
        invalid_indices =  np.where(np.isnan(accel_data)) # Remove headers and invalid data
        accel_data = np.delete(accel_data, invalid_indices, axis=0)
        return accel_data
    
    def load_all_data(self,folder_path):
        self.raw_calibration_data = []
        accel_list= np.zeros((M,self.N,3))
        i = 0
        for file_name in os.listdir(folder_path):
            if file_name.endswith(".csv"):
                file_path = os.path.join(folder_path, file_name)
                accel_data = self.load_data(file_path)
                accel_data= accel_data[:self.N]
                accel_list[i] = accel_data
                i+=1
        return accel_list

    def compute_T(self,alpha_yz, alpha_zy, alpha_zx):
        return np.array([
            [1, alpha_yz, -alpha_zy],
            [0, 1, alpha_zx],
            [0, 0, 1]
        ])
        
    def compute_R(self,phi, varphi):
        return np.array([
            [np.cos(varphi)*np.cos(phi), np.sin(varphi)*np.cos(phi), -np.sin(phi)],
            [-np.sin(varphi), np.cos(varphi), 0],
            [np.cos(varphi)*np.sin(phi), np.sin(varphi)*np.sin(phi), np.cos(phi)]
        ])

    def model_output(self,theta1, u):
        kx, ky, kz, alpha_yz, alpha_zy, alpha_zx, bx, by, bz = theta1
        K = np.diag([kx, ky, kz])
        T_inv = self.compute_T(alpha_yz, alpha_zy, alpha_zx)
        return K @ T_inv @ u + np.array([bx, by, bz])

    def orientation_residuals(self,theta2_i, theta1, data_i):
        phi, varphi = theta2_i
        # Simplified rotation matrix (assuming heading ψ=0)
        R = self.compute_R(phi, varphi)
        u = R @ np.array([0, 0, g])  # True specific force
        residuals = data_i - self.model_output(theta1, u)
        return residuals.flatten()

    def calibration_residuals(self,theta1, theta2, data):
        residuals = []
        for m in range(M):
            phi, varphi = theta2[2*m], theta2[2*m+1]
            R = self.compute_R(phi, varphi)
            u = R @ np.array([0, 0, g])
            y_pred = self.model_output(theta1, u)
            residuals.extend(data[m] - y_pred)
        return np.array(residuals).flatten()

    def calibrate(self,data_path):
        self.raw_calibration_data = self.load_all_data(data_path)
        theta1 = np.array([1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])  # [kx, ky, kz, α_yz, α_zy, α_zx, bx, by, bz]
        theta2 = np.zeros(2 * M)  # [phi_1, varphi_1, ..., phi_M, varphi_M]
        for iteration in range(self.max_iterations):
            for m in range(M):
                result = least_squares(
                    self.orientation_residuals,
                    theta2[2*m:2*m+2],
                    args=(theta1, self.raw_calibration_data[m]),
                    bounds=([-np.pi/2, -np.pi], [np.pi/2, np.pi])
                )
                theta2[2*m:2*m+2] = result.x
            
            result = least_squares(
                self.calibration_residuals,
                theta1,
                args=(theta2, self.raw_calibration_data)
            )
            theta1_new = result.x
            
            if np.linalg.norm(theta1_new - theta1) < self.tolerance:
                print(f"Converged after {iteration+1} iterations.")
                theta1 = theta1_new
                break
            theta1 = theta1_new
        else:
            print("Max iterations reached without convergence.")

        # Output results
        self.calibration_scale = theta1[:3]
        self.calibration_misalignment = theta1[3:6]
        self.calibration_bias = theta1[6:]
        print("\nCalibrated Parameters:")
        print(f"Scale factors: kx = {theta1[0]:.6f}, ky = {theta1[1]:.6f}, kz = {theta1[2]:.6f}")
        print(f"Misalignments: α_yz = {theta1[3]:.6f}, α_zy = {theta1[4]:.6f}, α_zx = {theta1[5]:.6f}")
        print(f"Biases: bx = {theta1[6]:.6f}, by = {theta1[7]:.6f}, bz = {theta1[8]:.6f}")

    
    def apply_calibration(self,data_path):
        raw_data = self.load_data(data_path)
        T_inv = np.array([
            [1, self.calibration_misalignment[0], -self.calibration_misalignment[1]],
            [0, 1, self.calibration_misalignment[2]],
            [0, 0, 1]
        ])
        T = np.linalg.inv(T_inv)
        K = np.diag([self.calibration_scale[0], self.calibration_scale[1], self.calibration_scale[2]])
        K_inv = np.diag(1 / np.array([self.calibration_scale[0],self.calibration_scale[1], self.calibration_scale[2]]))
        b = np.array([self.calibration_bias[0], self.calibration_bias[1], self.calibration_bias[2]])
        
        # Apply calibration to each sample
        calibrated_data = np.zeros_like(raw_data)
        for i in range(raw_data.shape[0]):
            calibrated_data[i] = T @ K_inv @ (raw_data[i] - b)
        
        return calibrated_data
    
    def filter_data(self,data):
        window_size = 8
        filtered_data = np.array([
            np.convolve(data[:, i], np.ones(window_size)/window_size, mode='valid') 
            for i in range(3)
        ]).T  # Transpose to maintain shape (N, 3)
        return filtered_data

    def compute_position(self,data,output_file):
        filtered_data = self.filter_data(data)
        positions = []
        self.acceleration = np.array([0.0, 0.0, 0.0])
        self.velocity = np.array([0.0, 0.0, 0.0])
        self.position = np.array([0.0, 0.0, 0.0])
        self.count = 0
        with open(output_file, 'w') as f:
            f.write('X,Y,Z\n')
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
                f.write(f'{self.position[0]},{self.position[1]},{self.position[2]}\n')
                positions.append(self.position.copy())
        
        return np.array(positions)

# Example usage
processor = PositionEstimator()
processor.calibrate('data/calibration')
calibrated_data = processor.apply_calibration('data/movement/movement.csv')
print(calibrated_data.shape)
calibrated_data_str = np.array([[f"{x:.2f}" for x in row] for row in calibrated_data])
np.savetxt(f'calibrated_data.csv', calibrated_data_str, delimiter=',', fmt='%s', header='X,Y,Z', comments='')
calibrated_pos = processor.compute_position(calibrated_data, 'calibrated_position.csv')
processor_2 = PositionEstimator()
raw_position = processor_2.load_data('data/movement/movement.csv')
raw_pos = processor_2.compute_position(raw_position, 'raw_position.csv')

# Plot raw vs calibrated position paths in x-y plane
plt.figure(figsize=(10, 8))

# Plot both paths on same axes
plt.plot(raw_pos[0, 0], raw_pos[0, 1], 'bo', label='Raw Start')
plt.plot(raw_pos[-1, 0], raw_pos[-1, 1], 'bx', label='Raw End')
plt.plot(raw_pos[:, 0], raw_pos[:, 1], 'b-', label='Raw Path')
plt.plot(calibrated_pos[0, 0], calibrated_pos[0, 1], 'ro', label='Calibrated Start') 
plt.plot(calibrated_pos[-1, 0], calibrated_pos[-1, 1], 'rx', label='Calibrated End')
plt.plot(calibrated_pos[:, 0], calibrated_pos[:, 1], 'r--', label='Calibrated Path')

plt.title('Position Paths in X-Y Plane')
plt.xlabel('X Position (m)')
plt.ylabel('Y Position (m)')
plt.legend()
plt.grid(True)
plt.axis('equal')  # Equal scaling on both axes
plt.show()
