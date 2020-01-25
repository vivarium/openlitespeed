# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>

# Adapted to UDNS by Luca Cantoreggi <luca@cantoreggi.xyz>

# - Find udns
# Find the native udns headers and libraries.
#
# UDNS_INCLUDE_DIRS	- where to find udns.h, etc.
# UDNS_LIBRARIES	- List of libraries when using udns.
# UDNS_FOUND	- True if udns found.

# Look for the header file.
FIND_PATH(UDNS_INCLUDE_DIR NAMES udns.h)

# Look for the library.
FIND_LIBRARY(UDNS_LIBRARY NAMES udns)

# Handle the QUIETLY and REQUIRED arguments and set UDNS to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UDNS DEFAULT_MSG UDNS_LIBRARY UDNS_INCLUDE_DIR)

# Copy the results to the output variables.
IF(UDNS_FOUND)
    SET(UDNS_LIBRARIES ${UDNS_LIBRARY})
    SET(UDNS_INCLUDE_DIRS ${UDNS_INCLUDE_DIR})
ELSE(UDNS_FOUND)
    SET(UDNS_LIBRARIES)
    SET(UDNS_INCLUDE_DIRS)
ENDIF(UDNS_FOUND)

MARK_AS_ADVANCED(UDNS_INCLUDE_DIRS UDNS_LIBRARIES)
