# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>

# Adapted to BROTLI by Luca Cantoreggi <luca@cantoreggi.xyz>

# - Find udns
# Find the native udns headers and libraries.
#
# BROTLI_INCLUDE_DIRS	- where to find udns.h, etc.
# BROTLI_LIBRARIES	- List of libraries when using udns.
# BROTLI_FOUND	- True if udns found.

# Look for the header file.
FIND_PATH(BROTLI_INCLUDE_DIR
        NAMES
            encode.h decode.h port.h types.h
        PATH_SUFFIXES
            brotli
)

# Look for the library.
FIND_LIBRARY(BROTLI_COMMON_LIBRARY
        NAMES
            brotlicommon
)

FIND_LIBRARY(BROTLI_ENCODE_LIBRARY
        NAMES
            brotlienc
)

FIND_LIBRARY(BROTLI_DECODE_LIBRARY
        NAMES
            brotlidec
)

# Handle the QUIETLY and REQUIRED arguments and set BROTLI to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BROTLI DEFAULT_MSG BROTLI_COMMON_LIBRARY BROTLI_INCLUDE_DIR)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BROTLI DEFAULT_MSG BROTLI_ENCODE_LIBRARY BROTLI_DECODE_LIBRARY BROTLI_INCLUDE_DIR)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BROTLI DEFAULT_MSG BROTLI_DECODE_LIBRARY BROTLI_INCLUDE_DIR)

# Copy the results to the output variables.
IF(BROTLI_FOUND)
    SET(BROTLI_LIBRARIES
            ${BROTLI_COMMON_LIBRARY}
            ${BROTLI_ENCODE_LIBRARY}
            ${BROTLI_DECODE_LIBRARY}
    )
    SET(BROTLI_INCLUDE_DIRS ${BROTLI_INCLUDE_DIR})
ELSE(BROTLI_FOUND)
    SET(BROTLI_LIBRARIES)
    SET(BROTLI_INCLUDE_DIRS)
ENDIF(BROTLI_FOUND)

MARK_AS_ADVANCED(BROTLI_INCLUDE_DIRS BROTLI_LIBRARIES)
