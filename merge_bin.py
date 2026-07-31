"""
Batch-merge ESP32-S3 firmware images for the Unimation BLE Mesh project.

For every serial-number NVS .bin found in the serial-number folder, this
script produces one fully self-contained merged firmware image containing:

    0x0      bootloader.bin
    0x8000   partition-table.bin
    0x1b000  <SERIAL>.bin        (unique per device)
    0x20000  app.bin             (e.g. IR_BLE_MESH.bin)

Usage:
    python merge_firmware.py

Adjust the paths in the CONFIG section below to match your project layout.
Requires esptool.py to be installed and on PATH (pip install esptool).
"""

import os
import subprocess
import sys

# ============================== CONFIG ==================================

PROJECT_DIR = r"D:\Freelance\Unimation\ble_mesh\ble_mesh_ur"

BOOTLOADER_BIN = os.path.join(PROJECT_DIR, "build", "bootloader", "bootloader.bin")
PARTITION_BIN  = os.path.join(PROJECT_DIR, "build", "partition_table", "partition-table.bin")
APP_BIN        = os.path.join(PROJECT_DIR, "build", "IR_BLE_MESH.bin")

# Folder containing one .bin per device serial number (from SerNoGen.py)
SERIAL_BIN_DIR = os.path.join(
    PROJECT_DIR, "main", "src", "NVS_Partition_Generator", "bin"
)

# Where to write the merged output files
OUTPUT_DIR = os.path.join(PROJECT_DIR, "merged_bin")

# Flash offsets (from ir_partitions.csv)
BOOTLOADER_OFFSET = "0x0"
PARTITION_OFFSET  = "0x8000"
SERIAL_OFFSET     = "0x1b000"
APP_OFFSET        = "0x20000"

CHIP        = "esp32s3"
FLASH_MODE  = "dio"
FLASH_FREQ  = "80m"
FLASH_SIZE  = "16MB"

# =========================================================================


def check_file(path, label):
    if not os.path.isfile(path):
        print(f"ERROR: {label} not found: {path}")
        sys.exit(1)


def main():
    check_file(BOOTLOADER_BIN, "Bootloader binary")
    check_file(PARTITION_BIN, "Partition table binary")
    check_file(APP_BIN, "App binary")

    if not os.path.isdir(SERIAL_BIN_DIR):
        print(f"ERROR: Serial bin directory not found: {SERIAL_BIN_DIR}")
        sys.exit(1)

    serial_bins = sorted(
        f for f in os.listdir(SERIAL_BIN_DIR)
        if f.lower().endswith(".bin")
    )

    if not serial_bins:
        print(f"ERROR: No .bin files found in {SERIAL_BIN_DIR}")
        sys.exit(1)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    print(f"Found {len(serial_bins)} serial-number binaries. Starting merge...\n")

    failures = []

    for serial_bin_name in serial_bins:
        serial_bin_path = os.path.join(SERIAL_BIN_DIR, serial_bin_name)
        device_name = os.path.splitext(serial_bin_name)[0]  # e.g. GWY00001
        output_path = os.path.join(OUTPUT_DIR, f"merged-{device_name}.bin")

        cmd = [
            sys.executable, "-m", "esptool",
            "--chip", CHIP,
            "merge_bin",
            "-o", output_path,
            "--flash_mode", FLASH_MODE,
            "--flash_freq", FLASH_FREQ,
            "--flash_size", FLASH_SIZE,
            BOOTLOADER_OFFSET, BOOTLOADER_BIN,
            PARTITION_OFFSET, PARTITION_BIN,
            SERIAL_OFFSET, serial_bin_path,
            APP_OFFSET, APP_BIN,
        ]

        print(f"[{device_name}] Merging -> {output_path}")
        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode != 0:
            print(f"  FAILED:\n{result.stderr}")
            failures.append(device_name)
        else:
            print(f"  OK")

    print("\n=== Summary ===")
    print(f"Total devices : {len(serial_bins)}")
    print(f"Succeeded     : {len(serial_bins) - len(failures)}")
    print(f"Failed        : {len(failures)}")
    if failures:
        print("Failed devices:", ", ".join(failures))
        sys.exit(1)

    print(f"\nAll merged binaries written to: {OUTPUT_DIR}")


if __name__ == "__main__":
    main()