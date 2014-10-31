#! /usr/bin/python
import os
import sys

# This funcions load the expected results into an array.
# The file must be already opened
#
# Return: A dictionary with test_names as keys and safe or unsafe depending on the example
def get_expected_answer(input_file):
	test_names = []
	test_results = []

	line = input_file.readline()
	while line != "":
		splitted_line = line.split()
		test_names.append(splitted_line[1])
		if (splitted_line[4] == "$\surd$"):
			test_results.append("safe")
		else:
			test_results.append("unsafe")
		line = input_file.readline()

	return dict(zip(test_names, test_results))


# This funcion read the next results from the file 'input_file'.
# The file must be already opened. The function lets the file ready to be used again by this same function
#
# Return a list with the test name and the result of a simple analysis. This result can be:
#	safe
#	unsafe
#	wrong
#   unuseless
def get_next_result(input_file):
	new_line = input_file.readline()
	flag = True #Let us set only one time the return value to avoid overwriting
	if new_line == "":
		return "end"

	test_name = os.path.splitext(os.path.basename(new_line.split()[1]))[0];
	ret = [test_name]

	new_line = input_file.readline()
	if "EEC" not in new_line:
		ret.append("unuseless")
		flag = False
	else:
		tmp_result = new_line.split()[2]
	if flag:
		new_line = input_file.readline()
		if "backward" not in new_line:
			ret.append("unuseless")
			flag = False
		else:
			if tmp_result != new_line.split()[3]:
				ret.append("wrong")
				flag = False
		if flag:
			new_line = input_file.readline()
			if "TSI" not in new_line:
				ret.append("unuseless")
				flag = False
			else:
				if tmp_result != new_line.split()[2]:
					ret.append("wrong")
					flag = False

		# At this point we have a correct result stored into variable tmp_result
		if flag:
			ret.append(tmp_result)

	# Advance to the next interesting line
	while "----------" not in new_line:
		new_line = input_file.readline()

	input_file.readline()
	return ret


########################################################################
# Start of the program

if len(sys.argv) != 3:
    print "Invalid imput format:"
    print "test_examples <results_to_check> <expected_results>"
    sys.exit(0)

results_to_check_file = open(sys.argv[1], "r")
expected_results_file = open(sys.argv[2], "r")

print "Getting expected results..."
expected_results = get_expected_answer(expected_results_file)

print "Comparing results..."
while True:
	result_to_check = get_next_result(results_to_check_file)

	if result_to_check == "end":
		break

	print "Test: ", result_to_check[0]

	if result_to_check[1] == "wrong":
		print "\033[31;01mSomething is wrong\n\033[00m"
	else:
		if result_to_check[1] == "unuseless":
			print "At least one of the tests was too slow\n"
		else:
			if result_to_check[0] not in expected_results.keys():
				print "\033[34;01mThere is no expected value for this example\n\033[00m"
			else:
				if expected_results[result_to_check[0]] == result_to_check[1]:
					print "\033[32;01mTest passed correctly\n\033[00m"
				else:
					print "\033[33;01mDismatch\n\033[00m"