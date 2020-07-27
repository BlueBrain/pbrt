#!/usr/bin/python
################################################################################
# Copyright BBP/EPFL (c) 2015
# Author : Marwan Abdellah (marwan.abdellah@epfl.ch)
################################################################################

import os
import sys
import subprocess
import glob

import sys
import os
import time
import math

# Globals
sl = "\n"      # Single new line
dl = "\n\n"    # Double new line
tl = "\n\n\n"  # Triple new line


###############################################################################
class SlurmConfiguration:
    """
    slurm configuration parameters.
    """
    def __init__(self):
        """
        * Initialization.
        """
        self.job_name = "VSD-sim"
        self.num_nodes = 1
        self.num_tasks_per_node = 1
        self.num_cpus_per_task = 16
        self.node_list = ""
        self.partition = "prod"
        self.memory_mb = "30000"
        self.session_time = "12:00:00"
        self.user_name = ""
        self.user_email = ""
        self.profile = ". /etc/profile"
        self.modules = "# No default modules are loaded"
        self.execution_path = ""
        self.env_path = ""
        self.env_ld_library_path = ""
        self.env_python_path = ""
        self.enable_email_notification = False
        self.enable_logs = True
        self.project_name = "proj3"
        
###############################################################################
def create_batch_config(slurm_config):
    """
    * creates the string that will be set into the batch job file.

    keyword arguments
    :param slurm_config : Slurm configuration parameters.
    :rtype              : String representing the batch job config. header.
    """

    # magic number
    b = "#!/bin/bash%s" % sl

    #########################
    # auto-generated header #
    #########################
    b += "######################################################%s" % sl
    b += "# WARNING - AUTO GENERATED FILE%s" % sl
    b += "# Please don't modify that file manually%s" % sl
    b += "######################################################%s" % sl

    ######################
    # node configuration #
    ######################
    # job name
    b += "#SBATCH --job-name=\"%s%d\"%s" % (slurm_config.job_name,
                                            slurm_config.job_number, sl)

    # number of nodes required to execute the job
    b += "#SBATCH --nodes=%s%s" % (slurm_config.num_nodes, sl)

    # number of cpus per tasks
    b += "#SBATCH --cpus-per-task=%s%s" % (slurm_config.num_cpus_per_task, sl)

    # number of tasks
    b += "#SBATCH --ntasks=%s%s" % (slurm_config.num_tasks_per_node, sl)

    # memory required per task in Mbytes
    b += "#SBATCH --mem=%s%s" % (slurm_config.memory_mb, sl)

    # slurm session time
    b += "#SBATCH --time=%s%s" % (slurm_config.session_time, sl)

    # job partition
    b += "#SBATCH --partition=%s%s" % (slurm_config.partition, sl)

    # job account
    b += "#SBATCH --account=%s%s" % (slurm_config.project_name, sl)

    # Reservation 
    # b += "#SBATCH --reservation=%s%s" % ('viz_team', sl)

    #####################
    # user notification #
    #####################
    if slurm_config.enable_email_notification:
        b += "#SBATCH --mail-type=ALL%s" % sl
        b += "#SBATCH --mail-user=%s%s" % (slurm_config.user_email, sl)

    ##################
    # log generation #
    ##################
    if slurm_config.enable_logs:
        std_out = "%s/slurm-stdout_%d.log" % \
                  (slurm_config.log_files_path, slurm_config.job_number)
        std_err = "%s/slurm-stderr_%d.log" % \
                  (slurm_config.log_files_path, slurm_config.job_number)
        b += "#SBATCH --output=%s%s" % (std_out, sl)
        b += "#SBATCH --error=%s%s" % (std_err, dl)

    ####################
    # System variables #
    ####################
    # slurm profile
    b += "# Loading profiles%s" % sl
    b += "%s%s" % (slurm_config.profile, dl)

    # job home
    b += "#JOB_HOME=\"%s\"%s" % (slurm_config.execution_path, sl)

    # KERBEROS renewal
    b += "# Renewal of KERBEROS periodically for the length of the job%s" % sl
    b += "krenew -b -K 30%s" % dl

    # slurm modules
    b += "# Loading the modules.%s" % sl
    b += "%s%s" % (slurm_config.modules, dl)
    b += "%s%s" % ("module load BBP/viz/latest", dl)

    # environmental variables
    b += "# Setting the environmental variables.%s" % sl
    b += "export PATH=%s:$PATH%s" % (slurm_config.env_path, sl)
    b += "export LD_LIBRARY_PATH=%s:$LD_LIBRARY_PATH%s" % \
         (slurm_config.env_ld_library_path, sl)
    b += "export PYTHONPATH=%s:$PYTHONPATH%s" % (slurm_config.env_python_path,
                                                 dl)
    # node list
    b += "echo \"On which node your job has been scheduled :\"%s" % sl
    b += "echo $SLURM_JOB_NODELIST%s" % dl

    # shell limits
    b += "echo \"Print current shell limits :\"%s" % sl
    b += "ulimit -a%s" % dl

    # running the serial tasks.
    b += "echo \"Now run your serial tasks ...\"%s" % sl
    b += "cd %s%s" % (slurm_config.execution_path, dl)
    ####################################################################

    return b


################################################################################
def list_files_in_directory(path, extension):
    """
    * lists all the files that ends with a specific extension in a single
    directory.

    keyword arguments
    :param path: the given directory
    :param extension: the extension of the file
    """

    directory_list = []
    for i_file in os.listdir(path):
        if i_file.endswith(".%s" % extension):
            directory_list.append(i_file)

    return directory_list
    
###############################################################################
def submit_batch_scripts(scripts_directory):
    """
    * lists all the generated slurm scripts and execute them one by one via
    the <sbatch> command.

    keyword arguments
    :param scripts_directory : where the batch scripts are generated.
    """

    # list all the scripts in the script directory.
    script_list = list_files_in_directory(scripts_directory, "sh")

    if len(script_list) == 0:
        print("There are no scripts to execute")

    # submit each script to the batch partition in the cluster.
    for script in script_list:

        # change the permissions
        shell_command = "chmod +x %s/%s" % (scripts_directory, script)
        subprocess.call(shell_command, shell=True)

        # execute the script
        shell_command = "sbatch %s/%s" % (scripts_directory, script)
        print(shell_command)

        # execute the shell command.
        subprocess.call(shell_command, shell=True)
