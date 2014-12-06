#! /usr/bin/python
#
# Author: Pedro Valero
#
# Description: Script used to check the correct functionallity of
# installed mist. (See help for more details)
#
# Date: 5/12/14
import os
import sys
import signal
import multiprocessing
import subprocess
import Queue

# This function represents a worker, which will be used by multiprocessing
# to determine the job that a subprocess should do
def worker(input, output):
    for test in iter(input.get, 'STOP'):
        worker_result = test + "*+*+"
        subprocesses = []
        for argument in ["--eec", "--backward", "--tsi", "--ic4pn"]:
            subprocesses.append(subprocess.Popen(['mist', argument, test], stdout=subprocess.PIPE, shell = False))

        for sp in subprocesses:
            worker_result += sp.communicate()[0]
        output.put(worker_result)


# This funcion read the next results from the file 'input_file'.
# The file must be already opened. The function lets the file ready to be used again by this same function
#
# Return a list with the test name and thre lists:
#   list of tests that match the expected output
#   list of tests that mismatch the expected output
#   list of tests whose result is unknown
def get_next_result(input_file):
    new_line = input_file.readline()
    flag = True #Let us set only one time the return value to avoid overwriting
    if new_line == "":
        return "end"

    test_full_name = new_line.split()[1]
    test_name = os.path.splitext(os.path.basename(test_full_name))[0];

    ret = [test_name]

    # Reading the expected result
    example_file = open(test_full_name,"r")
    results_comment = example_file.readline()
    if "expected" in results_comment:
        expected_result = results_comment.split()[2]
    else:
        ret.append("no_expected_result")
        expected_result = "safe"

    # List of tests whose output match the expected one
    match = []
    # List of tests whose output mismatch the expected one
    mismatch = []
    # List of tests whose output is unknown. At the begining all results are unknown
    unknown = ["eec", "backward", "tsi", "ic4pn"]

    not_executed = []

    num_timeouts = 0

    new_line = input_file.readline()

    while "----------" not in new_line:
        if "Timeout" in new_line:
            num_timeouts += 1
        if "EEC" in new_line:
            unknown.remove("eec")
            if new_line.split()[2] == "*":
                not_executed.append("eec")
            elif expected_result == new_line.split()[2]:
                match.append("eec")
            else:
                mismatch.append("eec")

        if "backward" in new_line:
            unknown.remove("backward")
            if expected_result == new_line.split()[3]:
                match.append("backward")
            else:
                mismatch.append("backward")

        if "TSI" in new_line:
            unknown.remove("tsi")
            if new_line.split()[2] == "*":
                not_executed.append("tsi")
            elif  expected_result == new_line.split()[2]:
                match.append("tsi")
            else:
                mismatch.append("tsi")

        if "ic4pn" in new_line:
            unknown.remove("ic4pn")
            if new_line.split()[2] == "*":
                not_executed.append("ic4pn")
            elif expected_result == new_line.split()[2]:
                match.append("ic4pn")
            else:
                mismatch.append("ic4pn")


        new_line = input_file.readline()

    input_file.readline()
    ret.append(match)
    ret.append(mismatch)
    ret.append(unknown)
    ret.append(not_executed)
    ret.append(num_timeouts)
    return ret


# This function collects all the examples from the folder folder_to_explore
# and subfolders and store it in the ouput_list
#
# Each element stored in the list is a path to an .spec example
# Returns: nothing
def list_files_with_spec_extension(folder_to_explore, output_list):
    list_all = os.listdir(folder_to_explore)
    for elem in list_all:
        full_name_elem = folder_to_explore+"/"+elem
        if os.path.isfile(full_name_elem) and elem != "README" and '.spec' in elem:
            output_list.append(full_name_elem)
        if os.path.isdir(full_name_elem):
            list_files_with_spec_extension(full_name_elem,output_list)


# This functions read all results from the file results_to_check_file
# and anlyze each of them, showing the results of the analysis by the
# standar ouput
#
# Returns: nothing
def analyze_results(results_to_check_file):
    results_to_check_file = open(results_to_check_file, "r")
    print "Comparing results..."
    while True:
        result_to_check = get_next_result(results_to_check_file)

        if result_to_check == "end":
            break

        if result_to_check[1] == "no_expected_result":
            print "Test ", result_to_check[0], "\033[34;01mThere is no expected value for this example\033[00m"
            safe = result_to_check[2]
            unsafe = result_to_check[3]
            unknown = result_to_check[4]

            if len(safe) != 0:
                print "\033[01m Safe: \033[00m", ", ".join(safe)
            if len(unsafe) != 0:
                print "\033[01m Unsafe: \033[00m", ", ".join(unsafe)
            if len(unknown) != 0:
                print "\033[01m Unknown: \033[00m", ", ".join(unknown)
            print ""

        else:
            match = result_to_check[1]
            mismatch = result_to_check[2]
            unknown = result_to_check[3]
            not_executed = result_to_check[4]
            num_timeouts = result_to_check[5]

            if len(match) + len(not_executed) == 4:
                print "Test ", result_to_check[0], "\033[32;01m OK\033[00m"
            else:
                if len(match) + len(unknown) == 4:
                    print "Test ", result_to_check[0], "\033[33;01m INCOMPLETE\033[00m"
                else:
                    print "Test ", result_to_check[0], "\033[31;01m ERR\033[00m"
                if len(match) != 0:
                    print "\033[01m Match: \033[00m", ", ".join(match)
                if len(mismatch) != 0:
                    print "\033[01m Mismatch: \033[00m", ", ".join(mismatch)
                if len(unknown) != 0:
                    print "\033[01m Unknown: \033[00m", ", ".join(unknown)
                    print "\033[01m \tTimeout: \033[00m", num_timeouts, " tests"
                if len(not_executed) != 0:
                    print "\033[01m Not executed: \033[00m", ", ".join(not_executed)
                print ""
    print "If there are more unknown results than timeouts then some example has crashed"


 # Function tu show help menu
def show_help():
    print "This script allows you to run a set of tests storing the results and analyzing them. The script also show you a summary about the correctness of the results obtained"
    print ""
    print "Usage: ./test_examples [--run|--all] [folder] [file] [number of subprocess]"
    print "Usage: ./test_examples [--analyze] [file]"
    print ""
    print "Options:"
    print "\t\033[01m--help\033[00m Shows this output"
    print "\t\033[01m--run\033[00m Runs mist on the examples in [folder] and writes the results to [file]"
    print "\t\033[01m--analyze\033[00m Compares the results stored in [file] against expected outcomes"
    print "\t\033[01m--all\033[00m Runs mist on the example in [folder], writes the results in [file] and compares against expected outcomes"
    print "This script will use [number of subprocess] process"



# This function checks if a tool called 'name' exists or not
#
# Returns:
#   True: The tool exists on this computer
#   False: The tool doesn't exsists on this computer
def is_tool(name):
    try:
        devnull = open(os.devnull)
        subprocess.Popen([name], stdout=devnull, stderr=devnull).communicate()
    except OSError as e:
        if e.errno == os.errno.ENOENT:
            return False
    return True


########################################################################
# Begin of the program

# Check if mist is instlled:
if is_tool("mist") == False:
    print "You should install mist before running this script"
    sys.exit(0)

run = False
analyze = False

# Parse input
#########################################################
if len(sys.argv) == 1 or sys.argv[1] == "--help":
    show_help()
else:
    if len(sys.argv) == 3:
        if sys.argv[1] == "--analyze":
            analyze = True
            output_file_name = sys.argv[2]
        else:
            print "Invalid imput format:"
            show_help()
            sys.exit(0)
    elif len(sys.argv) == 5:
        if sys.argv[1] == "--run":
            run=True
            output_file_name = sys.argv[3]
            folder = sys.argv[2]
        elif sys.argv[1] == "--all":
            run = True
            analyze = True
            output_file_name = sys.argv[3]
            folder = sys.argv[2]
        else:
            print "Invalid imput format:"
            show_help()
            sys.exit(0)
    else:
        print "Invalid imput format:"
        show_help()
        sys.exit(0)
#########################################################

if run:
    # Preparing the output file
    if os.path.isfile(output_file_name):
        print "The file ", output_file_name, " already exists. It will be overwritten."
        print "Do you want to continue anyways? [Y/N]"
        line = sys.stdin.readline()
        if line == "Y\n":
            print "The file will be overwritten"
        else:
            print "Exit"
            sys.exit(0)

    output_file = open(output_file_name, "w")
    max_number_of_subprocess = int(sys.argv[len(sys.argv)-1])

    print "This script runs the algorithms of mist on the files in [folder], output is collected in [file] and compared against expected outcomes. The script also outputs a summary of the results obtained. The max number of subprocess to be used is [number of subprocess]"


    #Empty list to store in it all the test files
    list_spec_files = []

    list_files_with_spec_extension(folder, list_spec_files)

    # We already have all the test stored in the variable 'list_spec_files'
    # Now we have to execute all of them and store the output

    # Create queues
    task_queue = multiprocessing.Queue()
    done_queue = multiprocessing.Queue()

    # Submit tasks
    for task in list_spec_files:
        task_queue.put(task)

    # Start worker processes
    for i in range(max_number_of_subprocess):
        multiprocessing.Process(target=worker, args=(task_queue, done_queue)).start()

    # Get and print results
    end = 0
    while end != len(list_spec_files):
        try:
            ic4pn = False
            conclusion = ""
            reachable = False
            process_output = done_queue.get()
            output_file.write("test: " + process_output.split('*+*+')[0] + "\n")
            for line in process_output.split('*+*+')[1].split("\n"):
                if "IC4PN" in line:
                    ic4pn = True # As Ic4pn will send so many information, we must know when we are analyzing its output
                if "concludes " in line:
                    if ic4pn:
                        conclusion = line #We must store the conclusion and take the last one
                    else:
                        output_file.write(line + "\n")
                if "Reachable " in line and ic4pn:
                    reachable = True
                if "Timeout" in line:
                    output_file.write(line + "\n")

                if conclusion != "":
                    output_file.write("ic4pn concludes" + conclusion.split("concludes")[1] + "\n") # Write the conclusion of ic4pn
                    conclusion = ""
                if reachable:
                    output_file.write("ic4pn concludes unsafe \n") # Write the conclusion of ic4pn
            output_file.write("-------------------------------------------------\n\n")
            end += 1
        except Queue.Empty:
            print "Some problem occurs"

    # Tell child processes to stop
    for i in range(max_number_of_subprocess):
        task_queue.put('STOP')

    output_file.close()
    print "Tests completed"

if analyze:
    analyze_results(output_file_name)

