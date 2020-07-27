#!/usr/bin/python 

# Imports 
import sys, os, subprocess

####################################################################################################
# @get_files_in_directory
####################################################################################################
def get_files_in_directory(directory,
                           file_extension=None):
    """
    Gets all the files in a directory, similar to ls command in linux. If the
    file extension is not specified, it returns a list of all the files.

    :param directory: Given directory.
    :param file_extension: The extension of requested file.
    :return: A list of the requested files.
    """

    # A list of all the files that exist in a directory
    files = []

    # If the extension is not specified
    if file_extension is None:
        for file in os.listdir(directory):
            files.append(file)

    # Otherwise, return files that have specific extensions
    else:
        for file in os.listdir(directory):
            if file.endswith(file_extension):
                files.append(file)

    # Return the list
    return files
    
# PBRT executable 
pbrt = '/home/abdellah/projects/bbp-pbrt-v2/build/bin/pbrt'

# Read the input file
input_pbrt_file = 'gaussian.pbrt.input'
input_pbrt_file_template = list()

# Open the file 
input_pbrt_file_handle = open(input_pbrt_file, 'r')

# Read it line by line 
for line in input_pbrt_file_handle:
    input_pbrt_file_template.append(line)

# Close the file 
input_pbrt_file_handle.close()

# Column y 
column_y = 3.13 

# Column height resolution 
volume_y = 313

# Step 
step = column_y / volume_y

# Number of steps 
n_steps = int(column_y / float(step))

# Output directory 
output_directory = 'output'

# Number of photons 
number_photons = 10000000

# Create an output file for each step
for i in range(0, 0):#n_steps + 1):
        
    # Output pbrt configuration 
    output_pbrt_file_data = list()
        
    # Depth 
    depth = str(i * step)
    
    # Prefix 
    prefix = '%s_%s' % (depth, str(number_photons))
     
    # Replace the parameters 
    for line in input_pbrt_file_template:
        if 'NUMBER_PHOTONS' in line:
            n_photons_line = line
            n_photons_line = n_photons_line.replace('NUMBER_PHOTONS', str(number_photons))
            output_pbrt_file_data.append(n_photons_line)
        elif 'OUTPUT' in line:
            output_line = line
            output_line = output_line.replace(
                'OUTPUT', '%s_depth%s_n%s' % (str(i), depth, str(number_photons)))
            output_pbrt_file_data.append(output_line)
        elif 'DEPTH' in line:
            depth_line = line
            depth_line = depth_line.replace('DEPTH', depth)
            output_pbrt_file_data.append(depth_line)
        else:
            output_pbrt_file_data.append(line)
    
    # Output file 
    output_pbrt_file = '%s/%s_%s.pbrt' % (output_directory, str(i), depth)
    print(output_pbrt_file)
    
    # Write the output file 
    output_pbrt_file_handle = open(output_pbrt_file, 'w')
    for line in output_pbrt_file_data:
        output_pbrt_file_handle.write(line)
    
    # Close the output file
    output_pbrt_file_handle.close()
    
    
# Get all the files in the directory 
pbrt_scripts = get_files_in_directory(output_directory, '.pbrt')

# Change directory 
os.chdir(output_directory)

# Execute them one by one 
for script in pbrt_scripts:
    
    # Execute the script 
    shell_command = '%s %s' % (pbrt, script)
    print(shell_command)
    subprocess.call(shell_command, shell=True) 
    
    
