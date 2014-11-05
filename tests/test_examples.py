#! /usr/bin/python
import os
import sys
import subprocess

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
    unknown = ["eec", "backward", "tsi"]

    new_line = input_file.readline()

    while "----------" not in new_line:
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
            if expected_result == new_line.split()[2]:
                match.append("tsi")
            else:
                mismatch.append("tsi")


        new_line = input_file.readline()

    input_file.readline()
    ret.append(match)
    ret.append(mismatch)
    ret.append(unknown)
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

            if len(match) == 3:
                print "Test ", result_to_check[0], "\033[32;01m OK\033[00m"
            else:
                print "Test ", result_to_check[0], "\033[31;01m ERR\033[00m"
                if len(match) != 0:
                    print "\033[01m Match: \033[00m", ", ".join(match)
                if len(mismatch) != 0:
                    print "\033[01m Mismatch: \033[00m", ", ".join(mismatch)
                if len(unknown) != 0:
                    print "\033[01m Unknown: \033[00m", ", ".join(unknown)
                print ""


 # Function tu show help menu
def show_help():
    print "This script allows you to run a set of tests storing the results and analyzing them. The script also show you a summary about the correctness of the results obtained"
    print ""
    print "Usage: ./test_examples [--options] [folder] [file]"
    print ""
    print "Options:"
    print "\t\033[01m--help\033[00m Shows this output"
    print "\t\033[01m--run\033[00m Runs the examples from the folder given and write a summary of the outputs in file"
    print "\t\033[01m--analyze\033[00m Analyzes the results stored into the file"
    print "\t\033[01m--all\033[00m Runs all the test from the folder, stores the results into file and analyze it"


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
    print "You should install mist before run this script"
    sys.exit(0)

run = False
analyze = False
# Parse input
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
    elif len(sys.argv) == 4:
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


# Mutex to sincronyse the thread when writing the output
if run:
    if os.path.isfile(output_file_name):
        print "The file ", output_file_name, " already exists. It would be overwritten."
        print "Do you want to continue anyways? [Y/N]"
        line = sys.stdin.readline()
        if line == "Y\n":
            print "The file will be overwritted"
        else:
            print "Exit"
            sys.exit(0)

    output_file = open(output_file_name, "w")

    print "This script execute mist tool whih all the examples contained in the foler given as argument whith extension .spec"

    #Empty list to store in it all the test files
    list_spec_files = []

    list_files_with_spec_extension(folder, list_spec_files)

    print list_spec_files

    # We already have all the test stored in the variable 'list_spec_files'
    # Now we have to execute all of them and store the output

    count = 0
    list_of_process = []
    for test in list_spec_files:
        count +=1
        print "\nRound:", count, "\n Wroking with test: ", test
        process = subprocess.Popen(['./execute_test.py', test], stdout=subprocess.PIPE)
        list_of_process.append(process)


    # Analyze results
    print "Waiting for the threads..."

    index = 0
    for p in list_of_process:
        output_file.write("test: " + list_spec_files[index] + "\n")
        process_output = p.communicate()[0].split('\n')
        for line in process_output:
            if ("concludes" in line):
                output_file.write(line + "\n")
        output_file.write("-------------------------------------------------\n\n")
        index += 1

    output_file.close()
    print "All given examples has been executed"

if analyze:
    analyze_results(output_file_name)

