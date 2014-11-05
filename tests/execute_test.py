#! /usr/bin/python
import os
import sys
import subprocess

# This scripts should not be called directly by user so won't check the
# received arguments

tmp_output = "tmp"
command = "mist "
tool_arguments = ["--eec ", "--backward ", "--tsi "]
test = sys.argv[1]

for argument in tool_arguments:
    print "Executing example with arguments: ", argument
    os.system(command + argument + test)