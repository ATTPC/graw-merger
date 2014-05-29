Command line usage
==================

..  highlight:: bash

The command `get-manip` can be called with a few different options. The basic usage of the command is::
    
    get-manip [--merge | --ls] [--lookup <lookup_table>] input [output]


Options
-------

..  program:: get-manip

..  option:: --merge
    
    Merge a set of GRAW files into one Event file.

..  option:: --ls

    List the contents of an Event file.

..  option:: --lookup <lookup_table>

    Provide a pad lookup table to the program. This is required when using the :option:`--merge` option.

..  option:: input

    The inputs to the program. This is either a directory of GRAW files or a single Event file.

..  option:: output

    The desired output file name and path. This is used with :option:`--merge`. If this option is not provided, the output will be written to the file `output.evt` in the present working directory.

Merging files
-------------

To merge many GRAW files into one Event file, `get-manip` should be called with the option :option:`--merge`. In this case, :option:`input` should be the path to a directory containing GRAW files, and :option:`output` should be the path to the desired output file. A comma-separated pad lookup table should be provided using :option:`--lookup`. 

Sample::

    get-manip --merge --lookup /path/to/lookup/table.csv /path/to/run/dir /path/to/output.evt 

Listing the contents of an Event file
-------------------------------------

The contents of an event file can be listed using the following syntax::

    get-manip --ls /path/to/eventfile.evt 