:class:`GETFrame` --- Frames from GRAW files
============================================

The class :class:`GETFrame` is used to represent the raw data frames produced by the GET electronics. It is constructed using a vector of raw data bytes extracted from a file. 

Expected input data format
--------------------------

The :class:`GETFrame` object expects the input data to be in the format of a GRAW file frame. This format is described at :doc:`GRAW`.

..	class:: GETFrame 

	..	function:: GETFrame(GETDataFile& file)

		Constructs a frame by pulling in the next raw frame from *file*, which must already be initialized. This automatically byte-swaps the raw integer values by calling a function in Utilities.h.

	..	function:: static uint8_t ExtractAgetId(const uint32_t raw)
		
		Extracts the AGET number from the compacted data item *raw*. 

	..	function:: static uint8_t ExtractChannel(const uint32_t raw)

		Extracts the channel number from the compacted data item *raw*.

	..	function:: static uint16_t ExtractTBid(const uint32_t raw)
		
		Extracts the time bucket ID from the compacted data item *raw*.

	..	function:: static int16_t ExtractSample(const uint32_t raw)

		Extracts the sample value from the compacted data item *raw*.
	    
	..	member:: private uint8_t metaType 

		Should equal 6.

	..	member:: private uint32_t frameSize

		The size of the frame. This is checked and corrected at run-time if it is wrong in the data.

	..	member:: private uint8_t dataSource

	..	member:: private uint16_t frameType

	..	member:: private uint8_t revision

	..	member:: private uint16_t headerSize

		The size of the header, in units of 64 bytes. Should be equal to 4. This is corrected at runtime if it is wrong.

	..	member:: private uint16_t itemSize

	..	member:: private uint32_t nItems

		The number of data items in this frame.

	..	member:: private uint64_t eventTime

		The timestamp of the frame. This should be the same for every input frame in a single event.

	..	member:: private uint32_t eventId

	..	member:: private uint8_t coboId

		The CoBo ID must be found from the file path since the current firmware sets this field to 0 for all CoBo boards.

	..	member:: private uint8_t asadId

		This is checked against the AsAd ID in the file name. The ID in the file name is preferred if they do not match.

	..	member:: private uint16_t readOffset

	..	member:: private uint8_t status

	..	member:: private std::vector< std::bitset<9*8> > hitPatterns
	
		..  TODO::
			Check these against the vector of traces.

	..	member:: private std::vector<uint8_t> multiplicity

	..	member:: private std::vector<GETFrameDataItem> data

		Each data item is stored in a :class:`GETFrameDataItem` object.

