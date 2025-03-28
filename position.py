import numpy as np
import matplotlib.pyplot as plt

def load_data_from_txt(file_path):
    return np.loadtxt(file_path, delimiter=' ')

def filter_data(data):
    window_size = 8 # you can change it if the data is too noisy or the movement is too slow
    filtered_data = np.array([
        np.convolve(data[:, i], np.ones(window_size)/window_size, mode='valid') 
        for i in range(3)
    ]).T  # Transpose to maintain shape (N, 3)
    return filtered_data

def compute_position(data,output_file):
    filtered_data = filter_data(data)
    positions = []
    acceleration = np.array([0.0, 0.0, 0.0])
    velocity = np.array([0.0, 0.0, 0.0])
    position = np.array([0.0, 0.0, 0.0])
    count = 0
    with open(output_file, 'w') as f:
        f.write('X,Y,Z\n')
        for acceleration in filtered_data:
            # Apply threshold to remove small variations (noise filtering)
            acceleration[np.abs(acceleration) <= 0.3] = 0  
            new_velocity = velocity + acceleration + (acceleration - acceleration) / 2 
            new_position = position + velocity + (new_velocity - velocity) / 2

            acceleration = acceleration
            velocity = new_velocity
            position = new_position

            # Movement end detection (if acceleration is consistently zero)
            if np.all(acceleration == 0):
                count += 1
            else:
                count = 0

            if count >= 25:
                velocity[:] = 0  # Stop velocity if no movement detected
                count = 0

            f.write(f'{position[0]},{position[1]},{position[2]}\n')
            positions.append(position.copy())
    
    return np.array(positions)

# Example usage
raw_data = load_data_from_txt('raw_data.txt')
raw_pos = compute_position(raw_data, 'calibrated_position.csv')

plt.figure(figsize=(10, 8))

plt.plot(raw_pos[0, 0], raw_pos[0, 1], 'bo', label='Raw Start')
plt.plot(raw_pos[-1, 0], raw_pos[-1, 1], 'bx', label='Raw End')
plt.plot(raw_pos[:, 0], raw_pos[:, 1], 'b-', label='Raw Path')

# calibrated_pos = compute_position(raw_data, 'calibrated_position.csv')
# plt.plot(calibrated_pos[0, 0], calibrated_pos[0, 1], 'ro', label='Calibrated Start') 
# plt.plot(calibrated_pos[-1, 0], calibrated_pos[-1, 1], 'rx', label='Calibrated End')
# plt.plot(calibrated_pos[:, 0], calibrated_pos[:, 1], 'r--', label='Calibrated Path')

plt.title('Position Paths in X-Y Plane')
plt.xlabel('X Position (m)')
plt.ylabel('Y Position (m)')
plt.legend()
plt.grid(True)
plt.axis('equal')
plt.show()
