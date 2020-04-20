#!/usr/bin/env python3

import concurrent.futures
import os
import requests
import sys
import time

# Functions

def usage(status=0):
    progname = os.path.basename(sys.argv[0])
    print(f'''Usage: {progname} [-h HAMMERS -t THROWS] URL
    -h  HAMMERS     Number of hammers to utilize (1)
    -t  THROWS      Number of throws per hammer  (1)
    -v              Display verbose output
    ''')
    sys.exit(status)

def hammer(url, throws, verbose, hid):
    ''' Hammer specified url by making multiple throws (ie. HTTP requests).

    - url:      URL to request
    - throws:   How many times to make the request
    - verbose:  Whether or not to display the text of the response
    - hid:      Unique hammer identifier

    Return the average elapsed time of all the throws.
    '''
    avg_time = []

    for i in throws:
        start_time = time.time()
        response = requests.get(url)
        if verbose:
            print(response.text)
        elapsed_time = time.time() - start_time
        avg_time.append(elapsed_time)

    total = 0
    for time in avg_time:
        total += avg_time[time]

    avg = total / len(avg_time)

    return avg

def do_hammer(args):
    ''' Use args tuple to call `hammer` '''
    return hammer(*args)

def main():
    hammers = 1
    throws  = 1
    verbose = False
    arguments = sys.argv[1:]

    # Parse command line arguments
    if len(sys.argv) == 1:
        usage(1)

    for index, arg in enumerate(arguments):
        if arg == '-t':
            throws = arguments.pop(index + 1)
        elif arg == '-h':
            hammers = arguments.pop(index + 1)
        elif arg == '-v':
            verbose = True
        elif arg.startswith('-'):
            usage(1)

    # Create pool of workers and perform throws
    with concurrent.futures.ProcessPoolExecutor(throws) as executor:
        result = executor.map(do_hammer, arguments)

# Main execution

if __name__ == '__main__':
    main()

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
