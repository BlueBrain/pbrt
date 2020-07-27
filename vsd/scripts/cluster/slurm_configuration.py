#!/usr/bin/python
################################################################################
# Copyright BBP/EPFL (c) 2015
# Author : Marwan Abdellah (marwan.abdellah@epfl.ch)
################################################################################



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
        self.num_cpus_per_task = 1
        self.node_list = ""
        self.partition = "prod"
        self.memory_mb = "2048"
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
