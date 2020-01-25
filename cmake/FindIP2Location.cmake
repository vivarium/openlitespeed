# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>

# Adapted to IP2LOCATION by Luca Cantoreggi <luca@cantoreggi.xyz>

# - Find IP2Location
# Find the native IP2Location headers and libraries.
#
# IP2LOCATION_INCLUDE_DIRS	- where to find IP2Location.h, etc.
# IP2LOCATION_LIBRARIES	- List of libraries when using IP2Location.
# IP2LOCATION_FOUND	- True if IP2Location found.

# Look for the header file.
FIND_PATH(IP2LOCATION_INCLUDE_DIR NAMES IP2Location.h)

# Look for the library.
FIND_LIBRARY(IP2LOCATION_LIBRARY NAMES IP2Location)

# Handle the QUIETLY and REQUIRED arguments and set IP2LOCATION to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IP2LOCATION DEFAULT_MSG IP2LOCATION_LIBRARY IP2LOCATION_INCLUDE_DIR)

# Copy the results to the output variables.
IF(IP2LOCATION_FOUND)
    SET(IP2LOCATION_LIBRARIES ${IP2LOCATION_LIBRARY})
    SET(IP2LOCATION_INCLUDE_DIRS ${IP2LOCATION_INCLUDE_DIR})
ELSE(IP2LOCATION_FOUND)
    SET(IP2LOCATION_LIBRARIES)
    SET(IP2LOCATION_INCLUDE_DIRS)
ENDIF(IP2LOCATION_FOUND)

MARK_AS_ADVANCED(IP2LOCATION_INCLUDE_DIRS IP2LOCATION_LIBRARIES)
