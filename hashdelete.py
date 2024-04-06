# Import module
import os
 
# Assign directory
directory = r"\managed_components"
 
# Iterate over files in directory
for folders in os.listdir(directory):
    search_directory = f"{directory}/{folders}"
    if os.path.isdir(folders):
        for files in os.listdir(search_directory):
            if files == ".component_hash":
                os.remove(f"{search_directory}/{files}")
                print(f"Deleted component_hash from {search_directory}/{files}")