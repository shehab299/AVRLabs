import numpy as np
import pandas as pd
import os
from scipy.spatial.transform import Rotation

# Calibration parameters
b_a = np.array([0.38395, -0.1313, 0.43695])  # in m/s²
K_a = np.diag([1.00173649, 1.00529907, 1.01810694])
R_a = np.array([[0.99895341, -0.00842065, 0.04495761],
                [0.01155954, 0.99992051, -0.00503448],
                [-0.05108319, 0.00527653, 0.99868046]])

file_orientations = [
    ("old/100hz_shehab/x_axis_neg.csv", [-9.81, 0, 0]),
    ("old/100hz_shehab/x_axis_pos.csv", [9.81, 0, 0]),
    ("old/100hz_shehab/y_axis_neg.csv", [0, -9.81, 0]),
    ("old/100hz_shehab/y_axis_pos.csv", [0, 9.81, 0]),
    ("old/100hz_shehab/z_axis_neg.csv", [0, 0, -9.81]),
    ("old/100hz_shehab/z_axis_pos.csv", [0, 0, 9.81])
]

def load_and_process(filepath, expected_g):
    try:
        df = pd.read_csv(filepath)
        
        accel = df[['aX (g)', 'aY (g)', 'aZ (g)']].values
        mean_accel = np.median(accel, axis=0)  # More robust than mean
        
        # Apply calibration
        calibrated = R_a.T @ np.linalg.inv(K_a) @ (mean_accel - b_a)
        
        return {
            'raw': mean_accel,
            'calibrated': calibrated,
            'expected': np.array(expected_g),
            'error': calibrated - expected_g,
            'error_norm': np.linalg.norm(calibrated - expected_g)
        }
    except Exception as e:
        print(f"Error processing {filepath}: {str(e)}")
        return None

# Process all files
results = []
for filepath, expected_g in file_orientations:
    if os.path.exists(filepath):
        result = load_and_process(filepath, expected_g)
        if result is not None:
            results.append(result)
    else:
        print(f"File not found: {filepath}")

if not results:
    raise ValueError("No valid data files processed")

# Compute metrics
errors = np.array([r['error'] for r in results])
error_norms = np.array([r['error_norm'] for r in results])

metrics = {
    'rms_error': np.sqrt(np.mean(errors**2)),
    'max_error': np.max(np.abs(errors)),
    'mean_error_norm': np.mean(error_norms),
    'scale_deviations': np.abs(np.diag(K_a) - 1) * 100,
    'misalignments': {
        'x_axis': np.degrees(np.arccos(np.clip(R_a[0,0], -1, 1))),
        'xy_angle': np.degrees(np.arccos(np.dot(R_a[0], R_a[1]))) - 90,
        'rotation': Rotation.from_matrix(R_a).magnitude() * 180/np.pi
    }
}

# Enhanced reporting
print("\n=== Comprehensive Calibration Validation ===")
print(f"\n1. Error Analysis (m/s²):")
print(f"   - RMS Error: {metrics['rms_error']:.6f}")
print(f"   - Max Error: {metrics['max_error']:.6f}")
print(f"   - Mean Error Norm: {metrics['mean_error_norm']:.6f}")

print("\n2. Scale Factor Deviations (%):")
for i, axis in enumerate(['X', 'Y', 'Z']):
    print(f"   - {axis}-axis: {metrics['scale_deviations'][i]:.4f}%")

print("\n3. Axis Misalignment:")
print(f"   - X-axis tilt: {metrics['misalignments']['x_axis']:.4f}°")
print(f"   - XY non-orthogonality: {metrics['misalignments']['xy_angle']:.4f}°")
print(f"   - Total rotation magnitude: {metrics['misalignments']['rotation']:.4f}°")

print("\n4. Per-Position Performance:")
print("   | Position | Raw (m/s²) | Calibrated (m/s²) | Error (m/s²) | Error Norm |")
print("   |----------|------------|-------------------|--------------|------------|")
for i, r in enumerate(results):
    print(f"   | {i+1:8} | [{r['raw'][0]:.3f}, {r['raw'][1]:.3f}, {r['raw'][2]:.3f}] | "
          f"[{r['calibrated'][0]:.3f}, {r['calibrated'][1]:.3f}, {r['calibrated'][2]:.3f}] | "
          f"[{r['error'][0]:.5f}, {r['error'][1]:.5f}, {r['error'][2]:.5f}] | {r['error_norm']:.5f} |")

