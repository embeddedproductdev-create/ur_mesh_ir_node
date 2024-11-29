import os
import subprocess
import argparse

def generate_nvs_binaries(device_type, start_serial, end_serial, output_dir, nvs_partition_gen_tool):
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    for i in range(start_serial, end_serial + 1):
        serial_number = f"{device_type.upper()}{str(i).zfill(5)}"
        csv_file = f"{output_dir}/nvs_serial_{i}.csv"
        bin_file = f"{output_dir}/nvs_serial_{i}.bin"
        
        # Create a CSV file for the current serial number
        with open(csv_file, "w") as f:
            f.write("key,type,encoding,value\n")
            f.write(f"SerialNo,data,string,{serial_number}\n")
        
        # Generate the NVS binary
        command = f"{nvs_partition_gen_tool} generate {csv_file} {bin_file} 0x6000"
        subprocess.run(command, shell=True, check=True)
        print(f"Generated: {bin_file}")

if __name__ == "__main__":
    # Define argument parser
    parser = argparse.ArgumentParser(description="Generate NVS binary files with unique serial numbers for ESP32 devices.")
    parser.add_argument("--deviceType", type=str, help="Device type prefix for serial numbers (e.g., GWY)", required=False)
    parser.add_argument("--start_serial", type=int, help="Starting serial number (e.g., 1)", required=False)
    parser.add_argument("--end_serial", type=int, help="Ending serial number (e.g., 100)", required=False)
    parser.add_argument("--output_dir", type=str, help="Directory to save the generated binary files", default="nvs_bins")
    parser.add_argument("--nvs_tool_path", type=str, help="Path to the NVS partition generator tool", 
                        default=r"python D:\Kulasekaran\MyProjects\IR\c01230-ir-ble-mesh-ac-controller-gwy\nvs_partition_gen.py")

    # Parse arguments
    args = parser.parse_args()

    # Prompt user for missing inputs
    device_type = args.deviceType or input("Enter device type (e.g., GWY): ").strip()
    start_serial = args.start_serial or int(input("Enter starting serial number: ").strip())
    end_serial = args.end_serial or int(input("Enter ending serial number: ").strip())
    output_dir = args.output_dir
    nvs_partition_gen_tool = args.nvs_tool_path

    # Validate inputs
    if start_serial > end_serial:
        print("Error: Start serial number must be less than or equal to the end serial number.")
        exit(1)

    # Display configuration summary
    print("\nConfiguration:")
    print(f"  Device Type: {device_type}")
    print(f"  Serial Number Range: {start_serial} to {end_serial}")
    print(f"  Output Directory: {output_dir}\n")

    # Generate NVS binaries
    generate_nvs_binaries(device_type, start_serial, end_serial, output_dir, nvs_partition_gen_tool)
