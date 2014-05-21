Event file
==========

The event file format is the usual output of the program. As opposed to the frame-centric structure of the GRAW file, the event file is structured around entire events. The events are written out in a somewhat similar format to the GRAW frames, however.

File header
-----------

At the very beginning of an event file, the following information is written:

+--------------+--------------+-------------------------------------+
|    Field     | Size (bytes) |             Description             |
+==============+==============+=====================================+
| Magic number |            4 | The value `0x6e7ef11e` (GETeFILE).  |
|              |              | Used to check if the file is of the |
|              |              | correct type.                       |
+--------------+--------------+-------------------------------------+

.. 	_event-header-desc:

Event header
------------

Like a GRAW frame, an event begins with a short header. 

+---------------+--------------+---------------------------------------------------+
|     Field     | Size (bytes) |                    Description                    |
+===============+==============+===================================================+
|               |              | The value `0xEE`. Used to check if the file       |
| Magic number  |            1 | pointer is at the beginning of an event.          |
+---------------+--------------+---------------------------------------------------+
| Size of event |            4 | The size of the entire event.                     |
+---------------+--------------+---------------------------------------------------+
| Event ID      |            4 | The index of the event, from the GRAW frame.      |
+---------------+--------------+---------------------------------------------------+
| Event Time    |            8 | The time stamp of the event, from the GRAW frame. |
+---------------+--------------+---------------------------------------------------+
| nTraces       |            2 | The number of traces in the event.                |
+---------------+--------------+---------------------------------------------------+

.. 	_trace-output-desc:

Traces
------

Each trace begins with these items:

+---------+--------------+-------------------------------------------+
|  Field  | Size (bytes) |                Description                |
+=========+==============+===========================================+
| size    |            4 | Size of the Trace, in bytes.              |
+---------+--------------+-------------------------------------------+
| coboId  |            1 | The CoBo ID for this trace.               |
+---------+--------------+-------------------------------------------+
| asadId  |            1 | The AsAd ID for this trace.               |
+---------+--------------+-------------------------------------------+
| agetId  |            1 | The AGET ID for this trace.               |
+---------+--------------+-------------------------------------------+
| channel |            1 | The channel of this trace.                |
+---------+--------------+-------------------------------------------+
| padId   |            2 | The pad number, as assigned in the        |
|         |              | lookup table provided during the creation |
|         |              | of the event file.                        |
+---------+--------------+-------------------------------------------+


After that, there are a series of time bucket / sample pairs. These are each compressed into 3 bytes.

+----------+----------------+--------------------+
|  Field   |   Bits used    |    Description     |
+==========+================+====================+
| TBid     | 24-16 (9 bits) | The time bucket.   |
+----------+----------------+--------------------+
| (Unused) | 15-13 (3 bits) | Padded with 0.     |
+----------+----------------+--------------------+
| Sample   | 12-0 (12 bits) | The sampled value. |
+----------+----------------+--------------------+

