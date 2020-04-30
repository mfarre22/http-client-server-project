#!/bin/sh

URL=student11.cse.nd.edu
PORT=9876
THOR=bin/thor.py
SMALL=1kb.bin
MED=1mb.bin
LARGE=1g.bin

# Test 1 small file (1KB)
echo "Test 1 - small file (1KB)"
./$THOR -t 10 http://$URL:$PORT/$SMALL

# Test 2 medium file (1MB)
echo "Test 2 - medium file (1MB)"
./$THOR -t 10 http://$URL:$PORT/$MED

# Test 3 large file (1GB)
echo "Test 3 - large file (1GB)"
./$THOR -t 10 http://$URL:$PORT/$LARGE
