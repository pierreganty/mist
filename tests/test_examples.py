#! /usr/bin/python
import os
import sys
import re
import subprocess
from operator import xor

# This function receives a file an determines if the net described in it is
# a Petri Net or doesn't
#
# Return
#   True: The net is a Petri Net
#   False: The net is not a Petri Net
def is_petri_net(test_file_name):
    test_file = open(test_file_name, "r")

    line = test_file.readline()
    regex_const = re.compile("[0-9]+") # regex to determine if we have a constant

    # This regex let us to split any effect of a rule into three kind of tokens:
    #  - strings
    #  - constants
    #  - empty tokens
    #
    # I.E.
    # The effect x0'=x0-1 will be splitted into :
    # [x0',x0,1] (And possible many empty tokens)
    delimiters = "+", "-", "'", "\n", "'", " ", "\t", "=", ","
    regexPattern = '|'.join(map(re.escape, delimiters))

    while line != "":
        if "->" in line:
            # For each rule, we group all effects into one string where each effect will be delimited by commas
            rule = line.split('->')[1]
            #If the rule is written along several lines
            if ";" not in rule:
                # Read lines until reach the end of the rule
                line = test_file.readline()
                while ";" not in line:
                    rule = rule + line
                    line = test_file.readline()
                # Add the last line, which contains ';'
                rule = rule + line

            # For each effect on the rule
            for effect in rule.split(','):
                # Split the effect into tokens according to what explained before
                splits_tmp = re.split(regexPattern, effect)
                #print splits_tmp

                #Removing empty tokens
                splits = [x for x in splits_tmp if x and x!=";"]
                #print splits

                if len(splits) != 3:
                    # If it was a petri net we should have 3 not empty tokens:
                    #   - New_variable'
                    #   - Old_variable'
                    #   - Constant
                    return False
                if (regex_const.match(splits[1]) is None and regex_const.match(splits[2]) is None) or (regex_const.match(splits[1]) is not None and regex_const.match(splits[2]) is not None):
                    return False

        line=test_file.readline()
    return True


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

    new_line = input_file.readline()

    while "----------" not in new_line:
        if "EEC" in new_line:
            unknown.remove("eec")
            if new_line.split()[2] == "*" or expected_result == new_line.split()[2]:
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
            if new_line.split()[2] == "*" or expected_result == new_line.split()[2]:
                match.append("tsi")
            else:
                mismatch.append("tsi")

        if "ic4pn" in new_line:
            unknown.remove("ic4pn")
            if new_line.split()[2] == "*" or expected_result == new_line.split()[2]:
                match.append("ic4pn")
            else:
                mismatch.append("ic4pn")


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

            if len(match) == 4:
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
                print ""


 # Function tu show help menu
def show_help():
    print "This script allows you to run a set of tests storing the results and analyzing them. The script also show you a summary about the correctness of the results obtained"
    print ""
    print "Usage: ./test_examples [--options] [folder] [file]"
    print ""
    print "Options:"
    print "\t\033[01m--help\033[00m Shows this output"
    print "\t\033[01m--run\033[00m Runs mist on the examples in [folder] and writes the results to [file]"
    print "\t\033[01m--analyze\033[00m Compares the results stored in [file] against expected outcomes"
    print "\t\033[01m--all\033[00m Runs mist on the example in [folder], writes the results in [file] and compares against expected outcomes"



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
        print "The file ", output_file_name, " already exists. It will be overwritten."
        print "Do you want to continue anyways? [Y/N]"
        line = sys.stdin.readline()
        if line == "Y\n":
            print "The file will be overwritten"
        else:
            print "Exit"
            sys.exit(0)

    output_file = open(output_file_name, "w")

    print "This script runs the algorithms of mist on the files in [folder], output is collected in [file] and compared against expected outcomes. The script also outputs a summary of the results obtained."

    #Empty list to store in it all the test files
    list_spec_files = []

    list_files_with_spec_extension(folder, list_spec_files)

    # We already have all the test stored in the variable 'list_spec_files'
    # Now we have to execute all of them and store the output

    count = 0
    list_of_process = []
    for test in list_spec_files:
        count += 1
        process = []
        print "\nExample:", count, "\n Working with test: ", test
        if (not is_petri_net(test)):
            tool_arguments = ["--backward "]
            process.append(subprocess.Popen('echo TSI concludes \*', stdout=subprocess.PIPE, shell = True))
            process.append(subprocess.Popen('echo EEC concludes \*', stdout=subprocess.PIPE, shell = True))
            process.append(subprocess.Popen('echo ic4pn concludes \*', stdout=subprocess.PIPE, shell = True))
        else:
            tool_arguments = ["--eec ", "--backward ", "--tsi ", "--ic4pn "]

        for argument in tool_arguments:
            print "Executing algorithm", argument, "of mist \n"
            process.append(subprocess.Popen('mist ' + argument + test, stdout=subprocess.PIPE, shell = True))


        list_of_process.append(process)

    # Analyze results
    print "mist is running. Please wait."

    index = 0

    for p in list_of_process:
        ic4pn = False
        conclusion = ""
        reachable = False
        output_file.write("test: " + list_spec_files[index] + "\n")
        for process_output_list in p:
            process_output = process_output_list.communicate()[0].split('\n')
            for line in process_output:
                if "IC4PN" in line:
                    ic4pn = True

                if "concludes " in line:
                    if ic4pn:
                        conclusion = line #We must store the conclusion and take the last one
                    else:
                        output_file.write(line + "\n")
                if "Reachable " in line:
                    reachable = True

            if conclusion != "":
                output_file.write("ic4pn concludes" + conclusion.split("concludes")[1] + "\n") # Write the conclusion of ic4pn
            if reachable:
                output_file.write("ic4pn concludes unsafe \n") # Write the conclusion of ic4pn
        output_file.write("-------------------------------------------------\n\n")
        index += 1

    output_file.close()
    print "Tests completed"

if analyze:
    analyze_results(output_file_name)

