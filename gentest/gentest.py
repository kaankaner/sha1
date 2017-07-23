#!/bin/python
# -*- coding: utf-8 -*- 


"""

This script was used to generate test cases for sha-1 algorithm.
Prints the test cases  to stdout.

Uses sha1sum as an external process.

"""

import getopt, sys, os, stat, subprocess
import random
import string


MAX_MSG_LEN = 120

# Generate this many test cases of each length (except 0).
NUM_EACH_LEN = 2

def call_sha1sum(msg):

	process = subprocess.Popen(['sha1sum'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)

	process.stdin.write(msg)
	process.stdin.close()
	
	# Get first line.
	for line in iter(process.stdout.readline, ''):
		output = line
		break

	#if output.endswith('\n'):
	#	output = output[:-1]

	# Remove the characters at the end.
	output = output[0:40]

	process.stdout.close();
	process.wait()

	return output


if __name__ == "__main__":

	# Always generate the same sequence.
	random.seed(1)

	print('static const auto& strpair = std::make_pair<std::string, std::string>;')
	print('typedef std::pair<std::string, std::string> StringPair;')
	print('extern const std::vector<StringPair> testCases = {')

	#print('const char* testCases[] = { ');

	for len in range(0, MAX_MSG_LEN):
		repeatCount = 1 if len == 0 else NUM_EACH_LEN
		for repeat in range(repeatCount):
			# Generate a string with len random characters drawn from the set of letters and digits.
			str = ''.join(random.choice(string.ascii_lowercase + string.ascii_uppercase + string.digits) for _ in range(len))
			digest = call_sha1sum(str)
			print('strpair("%s", "%s"),' % (digest, str) )

	print('};');

