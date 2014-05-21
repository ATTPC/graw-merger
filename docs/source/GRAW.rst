GRAW File --- Unprocessed output from the GET electronics
=========================================================

The GET electronics outputs one file per run per AsAd board. This means that if there are 10 CoBo boards and 4 AsAd boards for each CoBo, there should be 40 files per run. 

This program assumes that the input files will be organized in a certain way. Here is a sample directory structure:

* 	``20140323182024`` (Event directory. Name doesn't matter.)

	* 	``mm0`` (Directory for CoBo 0. The CoBo number is parsed from this.)

	 	*	``CoBo_AsAd0_2014-03-23T18/20/49.741_0000.graw`` 
	 		(File for AsAd 0 of CoBo 0. This AsAd number is checked against the one within the file.)
	 	* 	``CoBo_AsAd1_2014-03-23T18/20/49.812_0000.graw``
	 	* 	``CoBo_AsAd2_2014-03-23T18/20/49.814_0000.graw``
	 	* 	``CoBo_AsAd3_2014-03-23T18/20/49.815_0000.graw``

	* 	``mm1`` (Directory for CoBo 1.)

		* 	(One file per AsAd, as above)

	* 	Etc...

Each file consists of a series of "frames," where each frame represents one event seen by the detector. Since there are 10 CoBo boards and 4 AsAd boards per CoBo, there should be 40 frames per event, and these should come from 40 different files.

Format of a raw frame
---------------------

Each frame consists of a header (with information about the CoBo, AsAd, and event) followed by a series of data items (which contain the actual samples for each channel).

..	NOTE::
	The CoBo boards use a PowerPC-based architecture. This means that the raw data is encoded in a *big-endian* format, and will therefore need to be byte-swapped before it can be used on an Intel platform. This is done automatically during the construction of a GETFrame object.

Header
^^^^^^

+-------------+-------+---------------------------------------------------------+
|    Field    | Bytes |                       Description                       |
+=============+=======+=========================================================+
| metaType    |     1 | Should be 6                                             |
+-------------+-------+---------------------------------------------------------+
| frameSize   |     3 | Size of the frame in units of 64 bytes                  |
+-------------+-------+---------------------------------------------------------+
| dataSource  |     1 | ID of the source of the data (not used)                 |
+-------------+-------+---------------------------------------------------------+
| frameType   |     2 | Should be 1                                             |
+-------------+-------+---------------------------------------------------------+
| revision    |     1 | Currently at revision 4                                 |
+-------------+-------+---------------------------------------------------------+
| headerSize  |     2 | Size of the header, in units of 64 bytes.               |
|             |       | This is currently set to 4.                             |
+-------------+-------+---------------------------------------------------------+
| itemSize    |     2 | Size of a data item in bytes. Should be 4.              |
+-------------+-------+---------------------------------------------------------+
| nItems      |     4 | The number of data items in this frame.                 |
+-------------+-------+---------------------------------------------------------+
| eventTime   |     6 | Timestamp of this event.                                |
+-------------+-------+---------------------------------------------------------+
| eventId     |     4 | Index of this event. This may vary within a frame,      |
|             |       | even for the same event.                                |
+-------------+-------+---------------------------------------------------------+
| coboId      |     1 | ID of the CoBo this frame is for. This is currently     |
|             |       | set to 0 for all CoBos, and the correct CoBo number     |
|             |       | should be found from the filename.                      |
+-------------+-------+---------------------------------------------------------+
| asadId      |     1 | ID of the AsAd this event came from.                    |
+-------------+-------+---------------------------------------------------------+
| readOffset  |     2 | Index of the first time bucket that was read.           |
+-------------+-------+---------------------------------------------------------+
| status      |     1 | Unused, should be 0.                                    |
+-------------+-------+---------------------------------------------------------+
| hitPattern0 |     9 | 9-byte bit pattern of which pads were hit in this frame |
|             |       | for AGET 0.                                             |
+-------------+-------+---------------------------------------------------------+
| hitPattern1 |     9 | Same, but for AGET 1.                                   |
+-------------+-------+---------------------------------------------------------+
| hitPattern2 |     9 | Same, but for AGET 2.                                   |
+-------------+-------+---------------------------------------------------------+
| hitPattern3 |     9 | Same, but for AGET 3.                                   |
+-------------+-------+---------------------------------------------------------+
| multip_0    |     2 | AGET 0 multiplicity.                                    |
+-------------+-------+---------------------------------------------------------+
| multip_1    |     2 | AGET 1 multiplicity.                                    |
+-------------+-------+---------------------------------------------------------+
| multip_2    |     2 | AGET 2 multiplicity.                                    |
+-------------+-------+---------------------------------------------------------+
| multip_3    |     2 | AGET 3 multiplicity.                                    |
+-------------+-------+---------------------------------------------------------+

Data items
^^^^^^^^^^

To save space, each data item is compacted into 4 bytes (32 bits). These are arranged as follows:

+------------+----------------+------------------------------------------------------+
|   Field    |      Bits      |                     Description                      |
+============+================+======================================================+
| agetId     | 31-30 (2 bits) | ID of the AGET this data item came from. Range: 0-3. |
+------------+----------------+------------------------------------------------------+
| channel    | 29-23 (7 bits) | Input channel. Range: 0-67.                          |
+------------+----------------+------------------------------------------------------+
| timeBucket | 22-14 (9 bits) | Time bucket ID. Range: 0-511.                        |
+------------+----------------+------------------------------------------------------+
| (Unused)   | 13-12 (2 bits) | Padded with zeros.                                   |
+------------+----------------+------------------------------------------------------+
| Sample     | 11-0 (12 bits) | The sampled value.                                   |
+------------+----------------+------------------------------------------------------+


