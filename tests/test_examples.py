#! /usr/bin/python
import os
import sys

def list_files_with_spec_extension(folder_to_explore, output_list):
    list_all = os.listdir(folder_to_explore)
    for elem in list_all:
        full_name_elem = folder_to_explore+"/"+elem
        if os.path.isfile(full_name_elem) and elem != "README":
            output_list.append(full_name_elem)
        if os.path.isdir(full_name_elem):
            list_files_with_spec_extension(full_name_elem,output_list)


if len(sys.argv) != 3:
    print "Invalid imput format:"
    print "test_examples <folder> <output>"
    sys.exit(0)

folder = sys.argv[1]
output_file = open(sys.argv[2], "w")
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
for test in sorted(list_spec_files):
    count +=1
    print "Round:", count, "\n Wroking with test: ", test, "\n"
    output_file.write("test: " + test + "\n")

    for argument in tool_arguments:
        print "Executing example with arguments: ", argument
        os.system(command + argument + test + "> " + tmp_output)
        tmp_output_file = open(tmp_output)

        line = tmp_output_file.readline()
        while line != "":
          if ("concludes" in line):
            output_file.write(line)

          line = tmp_output_file.readline()

        tmp_output_file.close()
    output_file.write("-------------------------------------------------\n\n")

print "All examples given has been executed"
output_file.close()

