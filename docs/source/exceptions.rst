Exceptions thrown by the get-manip program
==========================================

This page lists exceptions that could be thrown by functions in this program. These are all defined in the header file `GMExceptions.h`, and they are contained in the namespace `Exceptions`. They are all derived from the standard library class `std::exception`.


Filesystem errors
-----------------

..	doxygenclass:: Exceptions::Bad_File
    :members: 

..  doxygenclass:: Exceptions::File_Open_Failed
	:members: 

..	doxygenclass:: Exceptions::Wrong_File_Type
    :members: 

.. 	doxygenclass:: Exceptions::Does_Not_Exist
	:members:  

..	doxygenclass:: Exceptions::Dir_is_Empty
	:members:  

I/O errors
----------

..	doxygenclass:: Exceptions::Frame_Read_Error
	:members:  

..	doxygenclass:: Exceptions::Wrong_File_Position
	:members:  

..	doxygenclass:: Exceptions::Bad_Data
	:members:  

..	doxygenclass:: Exceptions::End_of_File
	:members:  


Initialization errors
---------------------

..	doxygenclass:: Exceptions::Not_Init
	:members:  

..	doxygenclass:: Exceptions::Already_Init
	:members:  

Data container errors
---------------------

..  doxygenclass:: Exceptions::No_Data
	:members: 

Other errors
------------

..  doxygenclass:: Exceptions::Generic_Exception
	:members:  