# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>

# Adapted to MAXMIND by Luca Cantoreggi <luca@cantoreggi.xyz>

# - Find MAXMIND
# Find the native MAXMIND headers and libraries.
#
# MAXMIND_INCLUDE_DIRS	- where to find MAXMIND.h, etc.
# MAXMIND_LIBRARIES	- List of libraries when using MAXMIND.
# MAXMIND_FOUND	- True if MAXMIND found.

# Look for the header file.
FIND_PATH(MAXMIND_INCLUDE_DIR NAMES maxminddb.h)

# Look for the library.
FIND_LIBRARY(MAXMIND_LIBRARY NAMES maxminddb)

# Handle the QUIETLY and REQUIRED arguments and set MAXMIND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MAXMIND DEFAULT_MSG MAXMIND_LIBRARY MAXMIND_INCLUDE_DIR)

# Copy the results to the output variables.
IF(MAXMIND_FOUND)
    SET(MAXMIND_LIBRARIES ${MAXMIND_LIBRARY})
    SET(MAXMIND_INCLUDE_DIRS ${MAXMIND_INCLUDE_DIR})
ELSE(MAXMIND_FOUND)
    SET(MAXMIND_LIBRARIES)
    SET(MAXMIND_INCLUDE_DIRS)
ENDIF(MAXMIND_FOUND)

MARK_AS_ADVANCED(MAXMIND_INCLUDE_DIRS MAXMIND_LIBRARIES)
