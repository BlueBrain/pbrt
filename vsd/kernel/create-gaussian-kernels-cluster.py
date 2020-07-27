#!/usr/bin/python 

# Imports 
import sys, os, subprocess, shutil
import slurm


####################################################################################################
# @clean_and_create_new_directory
####################################################################################################
def clean_and_create_new_directory(path):
    """
    * creates a new directory after removing an existing one with the same name.

    keyword arguments
    :param path: the path to the created directory
    """

    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)
    
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
pbrt = '/gpfs/bbp.cscs.ch/project/proj3/development/bbp-pbrt-v2/build/bin/pbrt'

# Number of photons 
number_photons = 1000000000

# Output directories 
output_directory = '/gpfs/bbp.cscs.ch/project/proj3/simulations/vsd/gaussian-kernels/1e9'
pbrt_files_directory = '%s/%s' % (output_directory, 'pbrt')
results_directory = '%s/%s' % (output_directory, 'result')
clean_and_create_new_directory(output_directory)
clean_and_create_new_directory(pbrt_files_directory)
clean_and_create_new_directory(results_directory)

# Slurm directories
slurm_directory = '%s/%s' % (output_directory, 'slurm')
jobs_directory = '%s/%s' % (slurm_directory, 'jobs')
logs_directory = '%s/%s' % (slurm_directory, 'logs')
clean_and_create_new_directory(slurm_directory)
clean_and_create_new_directory(jobs_directory)
clean_and_create_new_directory(logs_directory)

# Read the input file
input_pbrt_file = '/gpfs/bbp.cscs.ch/project/proj3/development/bbp-pbrt-v2/vsd/kernel/kernel-template/gaussian.pbrt.input'
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
step = 0.05

# Number of steps 
n_steps = int(column_y / float(step))

# Create an output file for each step
for i in range(0, n_steps + 1):
        
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
                'OUTPUT', '%s/%s_depth%s_n%s' % (results_directory, str(i), depth, str(number_photons)))
            output_pbrt_file_data.append(output_line)
        elif 'DEPTH' in line:
            depth_line = line
            depth_line = depth_line.replace('DEPTH', depth)
            output_pbrt_file_data.append(depth_line)
        else:
            output_pbrt_file_data.append(line)
    
    # Output file 
    output_pbrt_file = '%s/%s_%s.pbrt' % (pbrt_files_directory, str(i), depth)
    print(output_pbrt_file)
    
    # Write the output file 
    output_pbrt_file_handle = open(output_pbrt_file, 'w')
    for line in output_pbrt_file_data:
        output_pbrt_file_handle.write(line)
    
    # Close the output file
    output_pbrt_file_handle.close()

    # Create the slurm configuration.
    slurm_configuration = slurm.SlurmConfiguration()
    slurm_configuration.job_number = i
    slurm_configuration.log_files_path = logs_directory

    # Create the slurm configuration string.
    slurm_batch_string = slurm.create_batch_config(slurm_configuration)

    # Create the execution command and then append it to the batch string
    shell_command = '%s %s' % (pbrt, output_pbrt_file)
    slurm_batch_string += shell_command
    
    # Save the script
    script_path = "%s/%s.sh" % (jobs_directory, str(i))    
    file_handler = open(script_path, 'w')
    file_handler.write(slurm_batch_string)
    file_handler.close()
    
# Run all the batch scripts
slurm.submit_batch_scripts(jobs_directory)
    
    
    
