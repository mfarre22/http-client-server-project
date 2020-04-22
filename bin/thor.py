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
    avg = []

    for i in range(throws):
        start = time.time()
        response = requests.get(url) # Time each request
        elapsed = time.time() - start
        avg.append(elapsed)
        print(f'Hammer: {hid}, Throw:   {i}, Elapsed Time: {elapsed:0.2f}')

        if verbose:
            print(response.text)

    total = 0
    for t in avg: # Calculate average time
        total += t

    average = total / len(avg)
    print(f'Hammer: {hid}, AVERAGE   , Elapsed Time: {average:0.2f}')

    return average

def do_hammer(args):
    ''' Use args tuple to call `hammer` '''
    return hammer(*args)

def main():
    hammers = 1
    throws  = 1
    dummy = 'f'
    verbose = False
    url = None
    arguments = sys.argv[1:]

    # Parse command line arguments
    if len(sys.argv) == 1:
        usage(1)

    for index, arg in enumerate(arguments):
        if arg == '-t':
            throws = int(arguments.pop(index + 1))
        elif arg == '-h':
            hammers = int(arguments.pop(index + 1))
        elif arg == '-v':
            arguments.pop(0)
            verbose = True
        elif arg.startswith('-'):
            usage(1)

    if arguments:
        url = arguments.pop()
    else:
        usage(1)

    # Create pool of workers and perform throws
    args = [(url, throws, verbose, hid) for hid in range(hammers)]

    with concurrent.futures.ProcessPoolExecutor(throws) as executor:
        result = list(executor.map(do_hammer, args))

    total = 0
    for r in result: # Calculate total average elapsed time
        total += r

    avg = total / len(result)

    print(f'TOTAL AVERAGE ELAPSED TIME: {avg:0.2f}')

# Main execution

if __name__ == '__main__':
    main()

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
