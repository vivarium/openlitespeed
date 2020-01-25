# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>

# Adapted to GEOIP by Luca Cantoreggi <luca@cantoreggi.xyz>

# - Find GEOIP
# Find the native GEOIP headers and libraries.
#
# GEOIP_INCLUDE_DIRS	- where to find GEOIP.h, etc.
# GEOIP_LIBRARIES	- List of libraries when using GEOIP.
# GEOIP_FOUND	- True if GEOIP found.

# Look for the header file.
FIND_PATH(GEOIP_INCLUDE_DIR NAMES GeoIP.h)

# Look for the library.
FIND_LIBRARY(GEOIP_LIBRARY NAMES GeoIP)

# Handle the QUIETLY and REQUIRED arguments and set GEOIP to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GEOIP DEFAULT_MSG GEOIP_LIBRARY GEOIP_INCLUDE_DIR)

# Copy the results to the output variables.
IF(GEOIP_FOUND)
    SET(GEOIP_LIBRARIES ${GEOIP_LIBRARY})
    SET(GEOIP_INCLUDE_DIRS ${GEOIP_INCLUDE_DIR})
ELSE(GEOIP_FOUND)
    SET(GEOIP_LIBRARIES)
    SET(GEOIP_INCLUDE_DIRS)
ENDIF(GEOIP_FOUND)

MARK_AS_ADVANCED(GEOIP_INCLUDE_DIRS GEOIP_LIBRARIES)
