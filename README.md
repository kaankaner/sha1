sha1 - an implementation of the SHA-1 algorithm written in cpp.

# COMPILATION

## LINKING WITH GTEST

If googletest is installed in your system, path variables related to 
googletest in the Makefile should be set to unassigned.

If googletest is not installed, it can be compiled like this:
apt update
apt install git cmake
mkdir lib
git clone https://github.com/google/googletest.git
cd googletest/googletest
mkdir build_static
cd build_static
cmake ..
make

## RUNNING

make test

