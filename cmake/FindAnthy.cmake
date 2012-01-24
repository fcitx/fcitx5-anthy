# - Try to find the ANTHY libraries
# Once done this will define
#
#  ANTHY_FOUND - system has ANTHY
#  ANTHY_INCLUDE_DIR - the ANTHY include directory
#  ANTHY_LIBRARIES - ANTHY library
#
# Copyright (c) 2012 CSSlayer <wengxt@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(ANTHY_INCLUDE_DIR AND ANTHY_LIBRARIES)
    # Already in cache, be silent
    set(ANTHY_FIND_QUIETLY TRUE)
endif(ANTHY_INCLUDE_DIR AND ANTHY_LIBRARIES)

find_package(PkgConfig)
pkg_check_modules(PC_LIBANTHY QUIET chewing)

find_path(ANTHY_MAIN_INCLUDE_DIR
          NAMES anthy.h
          HINTS ${PC_LIBANTHY_INCLUDEDIR}
          PATH_SUFFIXES anthy)

find_library(ANTHY_LIBRARIES
             NAMES anthy
             HINTS ${PC_LIBANTHY_LIBDIR})

set(ANTHY_INCLUDE_DIR "${ANTHY_MAIN_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ANTHY  DEFAULT_MSG  ANTHY_LIBRARIES ANTHY_MAIN_INCLUDE_DIR)

mark_as_advanced(ANTHY_INCLUDE_DIR ANTHY_LIBRARIES)
