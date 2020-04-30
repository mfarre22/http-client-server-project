#!/bin/sh

URL=student11.cse.nd.edu
THOR=bin/thor.py
PORT=9876
DIR=images
SCRIPT=scripts/hello.py
FILE=1mb.bin

# Test 1 - hammer, multiple throws - Directory
echo "Test 1 - Directory listing - single hammer, multiple throws"
./$THOR -t 10 http://$URL:$PORT/$DIR

# Test 2 - multiple hammers - Directory
echo "Test 2 - Directory listing - multiple hammers"
./$THOR -h 3 http://$URL:$PORT/$DIR

# Test 3 - multiple hammers, multiple throws - Directory
echo "Test 3 - Directory listing - multiple hammers, multiple throws"
./$THOR -h 3 -t 10 http://$URL:$PORT/$DIR

# Test 4 - hammer, multiple throws - Static files
echo "Test 4 - Static file - single hammer, multiple throws"
./$THOR -t 10 http://$URL:$PORT/$FILE

# Test 5 - multiple hammers - Static files
echo "Test 5 - Static file - multiple hammers"
./$THOR -h 3 http://$URL:$PORT/$FILE

# Test 6 - multiple hammers, multiple throws - Static files
echo "Test 6 - Static file - multiple hammers, multiple throws"
./$THOR -h 3 -t 10 http://$URL:$PORT/$FILE

# Test 7 - hammer, multiple throws - CGI script
echo "Test 7 - CGI script - single hammer, multiple throws"
./$THOR -t 10 http://$URL:$PORT/$SCRIPT

# Test 8 - multiple hammers - CGI script
echo "Test 8 - CGI script - multiple hammers"
./$THOR -h 3 http://$URL:$PORT/$SCRIPT

# Test 9 - multiple hammers, multiple throws - CGI script
echo "Test 9 - CGI script - multiple hammers, multiple throws"
./$THOR -h 3 -t 10 http://$URL:$PORT/$SCRIPT
