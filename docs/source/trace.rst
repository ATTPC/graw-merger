..	highlight:: cpp
	:linenothreshold: 5

:class:`Trace` --- Data for a single pad in an event
====================================================

A :class:`trace` object is used to represent the sampled output of a single pad in the detector in one event. It contains up to 512 samples, one for each time bucket. 

..	class:: Trace

	..	function:: Trace()
		
		The default constructor. It sets :member:`Trace::coboId`, :member:`Trace::asadId`, :member:`Trace::agetId`, :member:`Trace::channel`, and :member:`Trace::padId` to 0. This is primarily used for testing purposes.

	..	function:: Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad)

		Constructs an empty :class:`Trace` using the provided values.

		:param uint8_t cobo: The CoBo number.
		:param uint8_t asad: The AsAd number.
		:param uint8_t aget: The AGET number.
		:param uint8_t ch:   The channel.
		:param uint16_t pad: The pad number, as determined from a :class:`PadLookupTable`.

	..	function:: Trace(std::vector<uint8_t> raw)

		Constructs a :class:`Trace` from a vector of bytes that represent the serialized version of the :class:`Trace` object. This is used to recreate a :class:`Trace` from an Event File. The serialized format of the trace is given by the insertion operator below.

	..	function:: Trace(const Trace& orig)

		The copy constructor.

	..	function:: Trace(Trace&& orig)

		The move constructor.

	..	function:: Trace& operator=(const Trace& orig)

		The copy assignment operator.

	..	function:: Trace& operator=(Trace&& orig)

		The move assignment operator.

	..	function:: void AppendSample(int tBucket, int sample)

		Appends the value *sample* to the :class:`Trace` in time bucket *tBucket*. If that time bucket already has a sample in it, then there will be no change.

	..	function:: int16_t GetSample(int tBucket) const

		:return: The sample value in time bucket *tBucket*, if it exists.
		:exception: `std::out_of_range` if the element doesn't exist.

	..	function:: uint32_t size() const

		:return: The size of the entire :class:`Trace` object, in bytes.

	..	function:: unsigned long GetNumberOfTimeBuckets()

		:return: The number of time buckets filled.

	..	function:: Trace& operator+=(Trace& other)

		Adds the data of a :class:`Trace` *other* to the data of this :class:`Trace`. This is done by iterating over the time buckets and adding their contents, one by one. If this :class:`Trace` does not have a value in a particular time bucket, the value from *other* is inserted. The contents of *other* are not affected.

	..	function:: Trace& operator-=(Trace& other)

		Subtracts the data of a :class:`Trace` *other* from the data of this :class:`Trace`. This is done by iterating over the time buckets and subtracting their contents, one by one. If this :class:`Trace` does not have a value in a particular time bucket, then -1 times the value from *other* is inserted. The contents of *other* are not affected.

	..	function:: Trace& operator/=(Trace& other)

		Divides the data of this :class:`Trace` by the data of another :class:`Trace` *other*. This is done by iterating over the time buckets and dividing their contents, one by one. If this :class:`Trace` does not have a value in a particular time bucket, then nothing is done for that time bucket. The contents of *other* are not affected.

	..	function:: Trace& operator/=(int i)

		Divides the data of this :class:`Trace` by an integer *i*. This is done by iterating over the time buckets and dividing their contents by *i*, one by one. If this :class:`Trace` does not have a value in a particular time bucket, then nothing is done for that time bucket. 

	..	function:: void RenormalizeToZero()

		Renormalizes the data in this :class:`Trace` to 0. The mean of the data is found by adding the contents of all of the time buckets and dividing by the number of time buckets. This mean is then subtracted from each time bucket.

	..	function:: std::ostream& operator<<(std::ostream& stream, const Trace& trace)

		Writes the serialized contents of the :class:`Trace` to the output stream *stream*. Data is written in the order specified at :ref:`trace-output-desc`

	..	function:: static uint32_t CompactSample(uint16_t tb, int16_t val)
		
		Compacts a single time bucket (*tb*) / sample (*val*) pair into 3 bytes. This is done as follows::
		
			uint32_t joined = (tb << 15) | val;
			return joined;

		..	WARNING::
			Since the output from the electronics only contains 12-bit samples and 9-bit time buckets, this function assumes that these fields have at most those lengths. The time bucket will be truncated if it is wider than 9 bits, and the output will be unpredictable if the sample value is wider than 12 bits.

	..	function:: static std::pair<uint16_t,int16_t> UnpackSample(const uint32_t packed)

		Unpacks a compressed data sample, and returns a :class:`std::pair` that can then be appended to the data. This is done as follows::

			uint16_t tb = (packed & 0xFF8000) >> 15;
			int16_t val = packed & 0xFFF;
			std::pair<uint16_t,int16_t> res {tb,val};
			return res;

	.. 	member:: static const uint32_t sampleSize

		Represents the size of one compacted time bucket / sample pair. It equals 3 bytes.

	.. 	member:: uint8_t coboId

	.. 	member:: uint8_t asadId

	.. 	member:: uint8_t agetId

	.. 	member:: uint8_t channel

	.. 	member:: uint16_t padId

		The pad number, as determined by the :class:`PadLookupTable` provided when the event file was created.

	.. 	member:: std::map<uint16_t,int16_t> data

		The contained data. The data is stored in an associative map container, which is indexed by the time bucket number.



