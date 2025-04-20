from imucal import FerrarisCalibration, ferraris_regions_from_df
import pandas as pd
import numpy as np

folder = "../old/100hz_shehab"

x_axis_negative = f"./{folder}/x_axis_neg.csv"
x_axis_positive = f"./{folder}/x_axis_pos.csv"
y_axis_negative = f"./{folder}/y_axis_neg.csv"
y_axis_positive = f"./{folder}/y_axis_pos.csv"
z_axis_negative = f"./{folder}/z_axis_neg.csv"
z_axis_positive = f"./{folder}/z_axis_pos.csv"


file_paths = {
    "x_a": x_axis_negative,
    "y_a": y_axis_negative,
    "z_a": z_axis_negative,
    "x_p": x_axis_positive,
    "y_p": y_axis_positive,
    "z_p": z_axis_positive
}

data_chunks = []

for part, file in file_paths.items():
    df_part = pd.read_csv(file)[:100]
    df_part["part"] = part  # Add the part label
    data_chunks.append(df_part)

big_df = pd.concat(data_chunks, ignore_index=True)

# Rename columns
big_df = big_df.rename(columns={
    "aX (g)": "acc_x",
    "aY (g)": "acc_y",
    "aZ (g)": "acc_z",
    "gX": "gyr_x",
    "gY": "gyr_y",
    "gZ": "gyr_z"
})

big_df = big_df.drop(['Timestamp', 'Temp (C)'], axis=1)
num_rows = 1

rot_parts = ["x_rot", "y_rot", "z_rot"]
zero_data = np.zeros((num_rows * len(rot_parts), big_df.shape[1]))  # Ensure correct size

rot_df = pd.DataFrame(
    zero_data,
    columns=big_df.columns
)

rot_df["part"] = np.tile(rot_parts, num_rows)
big_df = pd.concat([big_df, rot_df])
big_df.set_index("part", inplace=True)

regions = ferraris_regions_from_df(big_df)

cal = FerrarisCalibration()

calibration_info = cal.compute(
    regions,
    100,
    'm/s^2',
    'deg/s'
)

Bias = calibration_info.b_a
Rotation_Matrix = calibration_info.R_a
Scale_Matrix = calibration_info.K_a

def acceleration_model(raw, calibration_info):

    Bias = calibration_info.b_a
    Rotation_Matrix = calibration_info.R_a
    Scale_Matrix = calibration_info.K_a

    return np.dot(np.linalg.inv(Scale_Matrix), np.dot(np.linalg.inv(Rotation_Matrix), raw - Bias))


