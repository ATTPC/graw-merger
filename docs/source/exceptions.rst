Exceptions thrown by the GETConsolidate program
===============================================

This page lists exceptions that could be thrown by functions in this program. These are all defined in the header file `GETExceptions.h`, and they are contained in the namespace `Exceptions`.

.. 	namespace:: Exceptions

Doxygen Version
---------------

.. 	doxygenclass:: Exceptions::GenericException
	:members: 

Filesystem errors
-----------------

..	class:: Exceptions::Bad_File : public std::exception

	Indicates that the program encountered an invalid file. 

	..  function:: Bad_File(const char* filename)

		The parameter *filename* is used to set the output string given by :func:`what()`.

	.. 	function:: const char* what() const

		:return: The string "Bad file: *filename*"

..	class:: Exceptions::Wrong_File_Type : public std::exception

	Thrown when a file of the wrong extension is encountered, or when the provided file does not contain any recognizable events or frames. 

	.. 	function:: Wrong_File_Type(const char* filename)

		The parameter *filename* is used to set the output string given by :func:`what()`.

	.. 	function:: const char* what() const

		:return: The string "Wrong file type: *filename*"

.. 	class:: Exceptions::Does_Not_Exist : public std::exception

	Thrown when a file or directory provided as an argument is not found on the filesystem.

	.. 	function:: Does_Not_Exist(const char* filename)

		The parameter *filename* is used to set the output string given by :func:`what()`.

	.. 	function:: const char* what() const

		:return: The string "File/Directory does not exist: *filename*"

..	class:: Exceptions::Dir_is_Empty : public std::exception

	Thrown if the input files directory provided to the program exists, but is empty.

	.. 	function:: Dir_is_Empty(const char* filename)

		The parameter *filename* is used to set the output string given by :func:`what()`.

	.. 	function:: const char* what() const

		:return: The string "This directory is empty: *filename*"

I/O errors
----------

..	class:: Exceptions::Frame_Read_Error : public std::exception

	Indicates that the program failed to read a raw frame from a GRAW file. In particular, this is thrown if the raw frame size is found to be zero. This may indicate a corrupted file or that the file cursor is at the wrong position.

	.. 	function:: const char* what() const

		:return: The string "Frame read failed."

..	class:: Exceptions::Wrong_File_Position : public std::exception

	Thrown if the file cursor is not correctly aligned with the beginning of a data item.

	.. 	function:: const char* what() const

		:return: The string "Item not found at this position."

..	class:: Exceptions::Bad_Data : public std::exception

	The data read was somehow corrupted or could not be parsed.

	.. 	function:: const char* what() const

		:return: The string "Corrupted or invalid data encountered."

..	class:: Exceptions::End_of_File : public std::exception

	Tried to read past the end of a file. This is thrown by the file classes.

	.. 	function:: const char* what() const

		:return: The string "Reached end of file."

Initialization errors
---------------------

..	class:: Exceptions::Not_Init : public std::exception

	An object that needed to be explicitly initialized before use was not.

	.. 	function:: const char* what() const

		:return: The string "Object not initialized."

..	class:: Exceptions::Already_Init : public std::exception

	An attempt was made to re-initialize an object that was already initialized.

	.. 	function:: const char* what() const

		:return: The string "Object already initialized."
