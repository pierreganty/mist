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
import os.path
import shutil

# This function represents a worker, which will be used by multiprocessing
# to determine the job that a subprocess should do
def worker(input, output):
    for test in iter(input.get, 'STOP'):
        worker_result = test + "*+*+"
        for argument in ["--eec", "--backward", "--tsi", "--ic4pn", "--cegar"]:
            if algorithm in argument:
                if graphs:
                    sp = subprocess.Popen(['mist', argument, test, '--timeout', str(timeout), '--graph', 'tmp_graphs/' + argument[2:] + "_" + os.path.splitext(os.path.basename(test))[0]+".csv"], stdout=subprocess.PIPE, shell = False, stderr=subprocess.PIPE)
                else :
                    sp = subprocess.Popen(['mist', argument, test, '--timeout', str(timeout)], stdout=subprocess.PIPE, shell = False, stderr=subprocess.PIPE)
                communication = sp.communicate()
                worker_result += communication[0]
                worker_result += communication[1]

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
    unknown = ["eec", "backward", "tsi", "ic4pn", "cegar"]

    not_executed = []

    num_timeouts = 0

    new_line = input_file.readline()

    not_petri_net = False

    while "----------" not in new_line:
        if "Not Petri Net" in new_line:
            not_petri_net = True
        if "Timeout" in new_line:
            num_timeouts += 1
        if "EEC" in new_line:
            unknown.remove("eec")
            if expected_result == new_line.split()[2]:
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
            if  expected_result == new_line.split()[2]:
                match.append("tsi")
            else:
                mismatch.append("tsi")

        if "ic4pn" in new_line:
            unknown.remove("ic4pn")
            if expected_result == new_line.split()[2]:
                match.append("ic4pn")
            else:
                mismatch.append("ic4pn")

        if "cegar" in new_line:
            unknown.remove("cegar")
            if expected_result == new_line.split()[2]:
                match.append("cegar")
            else:
                mismatch.append("cegar")


        new_line = input_file.readline()

    input_file.readline()
    if not_petri_net:
        unknown.remove("tsi")
        not_executed.append("tsi")
        unknown.remove("ic4pn")
        not_executed.append("ic4pn")
        unknown.remove("cegar")
        not_executed.append("cegar")
        unknown.remove("eec")
        not_executed.append("eec")
    ret.append(match)
    ret.append(mismatch)
    ret.append(unknown)
    ret.append(not_executed)
    ret.append(num_timeouts)
    return ret


# This function collects all the files.extension from the folder folder_to_explore
# and subfolders and store it in the ouput_list
#
# Each element stored in the list is a path to an .extension file
#
# Returns: nothing
def list_files_with_extension(folder_to_explore, output_list, extension):
    list_all = os.listdir(folder_to_explore)
    for elem in list_all:
        full_name_elem = folder_to_explore+"/"+elem
        if os.path.isfile(full_name_elem) and extension in elem:
            output_list.append(full_name_elem)
        if os.path.isdir(full_name_elem):
            list_files_with_extension(full_name_elem,output_list, extension)


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

            if len(match) + len(not_executed) == 5:
                print "Test ", result_to_check[0], "\033[32;01m OK\033[00m"
                print "Not a Petri Net, skipping eec, tsi and ic4pn"
            else:
                if len(match) + len(unknown) == 5:
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


 # Function tu show help menu
def show_help():
    print "This script allows you to run a set of tests storing the results and analyzing them. The script also show you a summary about the correctness of the results obtained"
    print ""
    print "Usage: ./run_benchmarks.py [--run|--all] [alg] [benchmarks directory] [output file] [number of subprocess] [timeout]"
    print "Usage: ./run_benchmarks.py [--analyze] [benchmarks directory] [output file]"
    print "Usage: ./run_benchmarks.py [--graphs] [alg] [benchmarks directory] [output folder] [number of subprocess] [timeout]"
    print ""
    print "Options:"
    print "\t\033[01m--help\033[00m Shows this output"
    print "\t\033[01m--run\033[00m Runs mist on the examples in [benchmarks directory] and writes the results to [output file]"
    print "\t\033[01m--analyze\033[00m Compares the results stored in [output file] against expected outcomes of the examples in [benchmarks directory]"
    print "\t\033[01m--all\033[00m Executes run + analyze"
    print "\t\033[01m--graphs\033[00m Runs mist on the examples in [benchmarks directory], using the algorithm [alg] and generating a folder [output folder] which contains a file [output folder].html with graphs of the memory usage of mist for the given examples. If algorithm '--' is specified all of them will be used."
    print "This script will use [number of subprocess] process and establish a timeout of [timeout] sec for each execution of mist (-1 to set off timeout)"



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
    print "You have to install mist before running this script"
    sys.exit(0)

run = False
analyze = False
graphs = False
algorithm = ""


# Parse input
#########################################################
if len(sys.argv) == 1 or sys.argv[1] == "--help":
    show_help()
else:
    if len(sys.argv) == 4:
        if sys.argv[1] == "--analyze":
            analyze = True
            folder = sys.argv[2]
            output_file_name = sys.argv[3]
        else:
            print "Invalid imput format:"
            show_help()
            sys.exit(0)
    elif len(sys.argv) == 7:
        if sys.argv[1] == "--graphs":
            algorithm = sys.argv[2]
            folder = sys.argv[3]
            output_graphs = sys.argv[4]
            graphs = True
            run = True
        elif sys.argv[1] == "--run":
            algorithm = sys.argv[2]
            folder = sys.argv[3]
            output_file_name = sys.argv[4]
            run = True
        elif sys.argv[1] == "--all":
            algorithm = sys.argv[2]
            folder = sys.argv[3]
            output_file_name = sys.argv[4]
            run = True
            analyze = True

        else:
            print "Invalid imput format:"
            show_help()
            sys.exit(0)
    else:
        print "Invalid imput format:"
        show_help()
        sys.exit(0)
#########################################################

if not os.path.isdir(folder):
    print "The origin folder does not exists. There is no data to work with"
    sys.exit(0)

if run:
    if not graphs:
        # Preparing the output file
        if os.path.isfile(output_file_name):
            print "The file ", output_file_name, "already exists. It will be overwritten."
            print "Do you want to continue anyways? [y/n]"
            line = sys.stdin.readline()
            if line == "y\n" or line =="Y\n":
                print "The file will be overwritten"
            else:
                print "Exit"
                sys.exit(0)

        output_file = open(output_file_name, "w")
    else:
        # Preparing the output folder
        if os.path.isdir(output_graphs):
            print "The folder", output_graphs, "alredy exists."
            print "You should remove it or choose another folder."
            sys.exit(0)

        print "Creating temporal files"
        os.makedirs("tmp_graphs")


    max_number_of_subprocess = int(sys.argv[len(sys.argv)-2])
    timeout = int(sys.argv[len(sys.argv)-1])

    print "Running the algorithms of mist on the files in", folder

    if not graphs:
        print "Output will be collected in ", output_file_name
    else:
        print "Graphs will be stored in ", output_graphs

    print "The max number of subprocess to be used is ",max_number_of_subprocess
    if timeout == -1:
        print "Timeout disabled\n"
    else:
        print "Timeout set up to ", timeout, "secs\n"

    #Empty list to store in it all the test files
    list_spec_files = []

    list_files_with_extension(folder, list_spec_files, ".spec")

    if list_spec_files == []:
        print "The folder ", folder, " does not contains any .spec file"
        sys.exit(0)

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

    if graphs:
        end = 0
        print "Running mist..."
        while end != len(list_spec_files):
            try:
                done_queue.get()
                end += 1
            except Queue.Empty:
                print "Some problem occurs"
        # Tell child processes to stop
        for i in range(max_number_of_subprocess):
            task_queue.put('STOP')

        print "Computation completed"
        list_csv_files = []

        list_files_with_extension("tmp_graphs", list_csv_files, ".csv")

        csv_files = ' '.join(list_csv_files)

        print "Preparing folder " + output_graphs
        os.makedirs(output_graphs)
        subprocess.call("./generate_graphs.sh "+ output_graphs + " " + csv_files, shell=True)

        print "Removing temporal folder"
        shutil.rmtree("tmp_graphs")
        sys.exit(0)

    # Get and print results
    end = 0
    while end != len(list_spec_files):
        try:
            ic4pn = False
            cegar = False
            conclusion = ""
            reachable = False
            process_output = done_queue.get()
            not_petri = False
            ic4pn_unsafe = False
            output_file.write("test: " + process_output.split('*+*+')[0] + "\n")
            for line in process_output.split('*+*+')[1].split("\n"):
                if "IC4PN" in line:
                    ic4pn = True # As Ic4pn will send so much information, we must know when we are analyzing its output
                if "CEGAR" in line:
                    cegar = True # As Cegar will send so much information, we must know when we are analyzing its output
                if "concludes " in line:
                    if ic4pn or cegar:
                        conclusion = line #We must store the conclusion and take the last one
                    else:
                        output_file.write(line + "\n")
                if "Reachable " in line and ic4pn:
                    reachable = True

                if conclusion != "":
                    if ic4pn:
                        output_file.write("ic4pn concludes" + conclusion.split("concludes")[1] + "\n") # Write the conclusion of ic4pn
                        ic4pn = False;
                    else:
                        output_file.write("cegar concludes" + conclusion.split("concludes")[1] + "\n") # Write the conclusion of cegar
                        cegar = False;
                    conclusion = ""

                if "Timeout" in line and not "established" in line:
                    output_file.write(line + "\n")
                if reachable:
                    ic4pn_unsafe = True
                if "The algorithm you selected only accepts Petri Net" in line:
                    not_petri = True

            if not_petri:
                output_file.write("Not Petri Net\n")
            if ic4pn_unsafe:
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
    if not os.path.isfile(output_file_name):
        print "The file", output_file_name, "does not exists"
        sys.exit(0)

    print "Analyzing results from ", output_file_name

    analyze_results(output_file_name)
