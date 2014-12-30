.. highlight:: bash

Introduction
============

Synopsis
--------

::

	get-manip -l <path> [-p <path>] [-t <value>] [-z] <input> [<output>]

Description
-----------

The `get-manip` command can be used to merge several GRAW files from the AT-TPC into one Event file. It indexes the GRAW files by event ID and then combines corresponding frames from all of the files into one event.

Options
-------

..  program:: get-manip

..  option:: -l,--lookup <path>

    Provide a pad lookup table to the program. This must have UNIX-style line endings and should list CoBo, AsAd, AGET, channel, and pad number on each line, separated by commas (a CSV file).

..  option:: -p,--pedestals <path>

	Subtract the pre-computed pedestals from the given file from the data. The provided path should point to a CSV file that lists one CoBo, AsAd, AGET, channel, and pedestal on each line.

..  option:: -t,--threshold <value>

	Apply the given threshold to the data. Any value below this threshold will be set to zero.

..  option:: -z,--zerosupp

	Suppress any zeros in the output file. This is best used with -p and -t, and will reduce the output file size greatly.

Output file naming
------------------

If no output file name is provided, the default is to create a file with the name of the input directory. For instance, if the input is at `/data/run_0001/`, then the output will be at `/data/run_0001.evt`.

Examples
--------

Assume we have several GRAW files in the directory `/data/run_0001` and lookup and pedestals files in the home directory. 

To just merge these files, run::

    get-manip -l ~/lookup_table.csv /data/run_0001

To instead merge them and write the data to /data/merged/example.evt, run::

	get-manip -l ~/lookup_table.csv /data/run_0001 /data/merged/example.evt

To merge, subtract pedestals, apply a threshold of 40, and suppress zeros, run::

	get-manip -l ~/lookup_table.csv -p ~/pedestals.csv -t 40 -z /data/run_0001
