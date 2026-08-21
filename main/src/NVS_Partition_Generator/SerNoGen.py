# =============================================================================
# USAGE - NVS Serial Number Generator for ESP32 BLE Mesh Devices
# =============================================================================
#
# Serial number prefix = first character of --deviceType
#   NODE → N prefix : N00000, N00001 ... N00100
#
# NODE bins (N00000 to N00100):
#   python SerNoGen.py --deviceType NODE --start_serial 0 --end_serial 100
#
# OUTPUT:
#   .bin files saved to 'bin' subfolder alongside this script.
#   Each .bin is flashed at offset 0x1b000 (serial NVS partition).
#   File naming: <prefix><5-digit-number>.bin  e.g. N00001.bin
#
# AFTER GENERATING NODE BINS:
#   Testing  : python node_merge_test.py N00001     (single device)
#              python node_merge_test.py             (all devices)
#   Release  : python node_merge_release.py          (versioned, requires git tag)
#
# PREREQUISITES:
#   1. Build the node firmware (IS_GWY=0) in Espressif IDE before merging.
#   2. For release builds, commit all changes and create a git tag first:
#          git tag -a v1.1.3 -m "description"
#          git push origin v1.1.3
#
# NOTE:
#   Regenerating will overwrite existing .bin and .csv files in the bin folder.
#   Devices already flashed must be reflashed with the new bins.
#   The bin folder is excluded from git via .gitignore.
# =============================================================================

import os
import subprocess
import argparse

def generate_nvs_binaries(device_type, start_serial, end_serial, output_dir, nvs_partition_gen_tool):
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    for i in range(start_serial, end_serial + 1):
        serial_number = f"{device_type.upper()[0]}{str(i).zfill(5)}"
        csv_file = f"{output_dir}/{serial_number}.csv"
        bin_file = f"{output_dir}/{serial_number}.bin"
        
        # Create a CSV file for the current serial number
        with open(csv_file, "w") as f:
            f.write("key,type,encoding,value\n")
            f.write("serial,namespace,,,\n")
            f.write(f"SerialNo,data,string,{serial_number}\n")
        
        # Generate the NVS binary
        command = f"{nvs_partition_gen_tool} generate {csv_file} {bin_file} 0x3000"
        subprocess.run(command, shell=True, check=True)
        print(f"Generated: {bin_file}")

if __name__ == "__main__":
    # Define argument parser
    parser = argparse.ArgumentParser(description="Generate NVS binary files with unique serial numbers for ESP32 devices.")
    parser.add_argument("--deviceType", type=str, help="Device type prefix for serial numbers (e.g., GWY, NODE)", required=False, default=None)
    parser.add_argument("--start_serial", type=int, help="Starting serial number (e.g., 0)", required=False, default=None)
    parser.add_argument("--end_serial", type=int, help="Ending serial number (e.g., 100)", required=False, default=None)
    parser.add_argument("--output_dir", type=str, help="Directory to save the generated binary files", default=os.path.join(os.path.dirname(__file__), "bin"))
    parser.add_argument("--nvs_tool_path", type=str, help="Path to the NVS partition generator tool",
                        default="python " + os.path.join(os.path.dirname(__file__), "nvs_partition_gen.py"))

    # Parse arguments
    args = parser.parse_args()

    # Prompt user for missing inputs.
    # Use 'is None' check instead of truthiness so --start_serial 0 works correctly
    # (0 is falsy in Python, so 'args.start_serial or input(...)' would prompt even when 0 is passed)
    device_type   = args.deviceType    if args.deviceType    is not None else input("Enter device type (e.g., GWY, NODE): ").strip()
    start_serial  = args.start_serial  if args.start_serial  is not None else int(input("Enter starting serial number: ").strip())
    end_serial    = args.end_serial    if args.end_serial    is not None else int(input("Enter ending serial number: ").strip())
    output_dir           = args.output_dir
    nvs_partition_gen_tool = args.nvs_tool_path

    # Validate inputs
    if start_serial > end_serial:
        print("Error: Start serial number must be less than or equal to the end serial number.")
        exit(1)

    # Display configuration summary
    print("\nConfiguration:")
    print(f"  Device Type   : {device_type.upper()}")
    print(f"  Serial Range  : {start_serial} to {end_serial} ({end_serial - start_serial + 1} devices)")
    print(f"  Output Dir    : {output_dir}\n")

    # Generate NVS binaries
    generate_nvs_binaries(device_type, start_serial, end_serial, output_dir, nvs_partition_gen_tool)