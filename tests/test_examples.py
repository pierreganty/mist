#! /usr/bin/python
import os
import sys
import multiprocessing
import threading

# Function to execute a test.
# This function receives the name of the file to print there the results and a mutex
# to keep sincronysed with other threads
#
# Returns: nothin
def execute_test(output, mutex, command, test, tool_arguments):
    tmp_file_name = "thread" + str(threading.current_thread().ident)

    for argument in tool_arguments:
        print "Executing example with arguments: ", argument
        os.system(command + argument + test + ">> " + tmp_file_name)

    tmp_output_file = open(tmp_file_name, "r")

    line = tmp_output_file.readline()
    mutex.acquire()
    output_file = open(output, "a")
    output_file.write("test: " + test + "\n")
    while line != "":
        if ("concludes" in line):
            output_file.write(line)

        line = tmp_output_file.readline()
    output_file.write("-------------------------------------------------\n\n")
    output_file.close()
    os.remove(tmp_file_name)
    mutex.release()
    tmp_output_file.close()


# This funcion read the next results from the file 'input_file'.
# The file must be already opened. The function lets the file ready to be used again by this same function
#
# Return a list with the test name and the result of a simple analysis. This result can be:
#   match
#   dismatch
#   wrong
#   unuseless
def get_next_result(input_file):
    new_line = input_file.readline()
    flag = True #Let us set only one time the return value to avoid overwriting
    if new_line == "":
        return "end"

    test_full_name = new_line.split()[1]
    test_name = os.path.splitext(os.path.basename(test_full_name))[0];

    ret = [test_name]

    new_line = input_file.readline()
    if "EEC" not in new_line:
        ret.append("useless")
        flag = False
    else:
        tmp_result = new_line.split()[2]
    if flag:
        new_line = input_file.readline()
        if "backward" not in new_line:
            ret.append("useless")
            flag = False
        else:
            if tmp_result != new_line.split()[3]:
                ret.append("wrong")
                flag = False
        if flag:
            new_line = input_file.readline()
            if "TSI" not in new_line:
                ret.append("useless")
                flag = False
            else:
                if tmp_result != new_line.split()[2]:
                    ret.append("wrong")
                    flag = False

        # At this point we have a correct result stored into variable tmp_result
        if flag:
            example_file = open(test_full_name,"r")
            expected_result = example_file.readline()
            if "expected" in expected_result:
                if tmp_result == expected_result.split()[2]:
                    ret.append("match")
                else:
                    ret.append("dismatch")
            else:
                ret.append("unknown")

    # Advance to the next interesting line
    while "----------" not in new_line:
        new_line = input_file.readline()

    input_file.readline()
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
        if os.path.isfile(full_name_elem) and elem != "README":
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

        print "Test: ", result_to_check[0]

        if result_to_check[1] == "wrong":
            print "\033[31;01mSomething is wrong\n\033[00m"
        else:
            if result_to_check[1] == "useless":
                print "At least one of the tests hadn't been completed\n"
            else:
                if result_to_check[1] == "unknown":
                    print "\033[34;01mThere is no expected value for this example\n\033[00m"
                else:
                    if result_to_check[1] == "match":
                        print "\033[32;01mTest passed correctly\n\033[00m"
                    else:
                        print "\033[33;01mDismatch\n\033[00m"


def show_help():
    print "This script allows you to run a set of tests storing the results and analyzing them. The script also show you a summary about the correctness of the results obtained"
    print ""
    print "Usage: ./test_examples [--options] [folder] [file]"
    print ""
    print "Options:"
    print "\t\033[01m--help\033[00m -Show this output"
    print "\t\033[01m--run\033[00m -Run the examples from the folder given and write a summary of the outputs in file"
    print "\t\033[01m--analyze\033[00m -Analyze the results stored into the file"
    print "\t\033[01m--all\033[00m -Run all the test from the folder, stores the results into file and analyze it"


########################################################################
# Begin of the program

run = False
analyze = False
# Parse input
if len(sys.argv) == 1 or sys.argv[1] == "--help":
    show_help()
else:
    if len(sys.argv) == 3:
        if sys.argv[1] == "--analyze":
            analyze = True
            output_file = sys.argv[2]
        else:
            print "Invalid imput format:"
            show_help()
            sys.exit(0)
    elif len(sys.argv) == 4:
        if sys.argv[1] == "--run":
            run=True
            output_file = sys.argv[3]
            folder = sys.argv[2]
        elif sys.argv[1] == "--all":
            run = True
            analyze = True
            output_file = sys.argv[3]
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
    mutex = threading.Lock()

    tmp_output = "tmp"
    command = "mist "
    tool_arguments = ["--eec ", "--backward ", "--tsi "]

    print "This script execute mist tool whih all the examples contained in the foler given as argument whith extension .spec"

    #Empty list to store in it all the test files
    list_spec_files = []

    list_files_with_spec_extension(folder, list_spec_files)

    # We already have all the test stored in the variable 'list_spec_files'
    # Now we have to execute all of them and store the output

    count = 0
    list_of_threads = []
    for test in sorted(list_spec_files):
        count +=1
        print "\nRound:", count, "\n Wroking with test: ", test

        thread = threading.Thread(target=execute_test, args=[output_file, mutex, command, test, tool_arguments])
        thread.start()
        list_of_threads.append(thread)


    # Analyze results
    print "Waiting for the threads..."
    [x.join() for x in list_of_threads]

    print "All examples given has been executed"

if analyze:
    analyze_results(output_file)

