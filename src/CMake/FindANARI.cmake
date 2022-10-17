# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#*****************************************************************************

#[=======================================================================[.rst:
FindANARI
---------

Finds the ANARI library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``anari::anari``
  The ANARI library
``anari::anari_utilities``
  The ANARI utilities library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``ANARI_FOUND``
  True if the system has the ANARI library.
``HAVE_ANARI_EXAMPLE``
  True if the ANARI example back-end library was found.
``HAVE_ANARI_VISRTX``
  True if the VisRTX back-end library was found.
``HAVE_ANARI_USD``
  True if the USD back-end library was found.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``anari_DIR``
  The directory containing the ANARI config files.
``ANARI_INCLUDE_DIRS``
  The directory containing ``anari/anari.h``.
``ANARI_LIBRARIES``
  The ANARI Libraries and third-party ANARI back-end libraries if found
``ANARI_Example_LIBRARY``
  The path for the ANARI Example back-end library
``ANARI_VISRTX_LIBRARY``
  The path for the ANARI VISRTX back-end library
``ANARI_USD_LIBRARY``
  The path for the ANARI USD back-end library

#]=======================================================================]

IF(EXISTS ${VISIT_ANARI_DIR})
    MESSAGE(STATUS "Checking for ANARI in ${ANARI_DIR}/lib/cmake/anari-${ANARI_VERSION}")

    IF(NOT DEFINED anari_DIR)
        SET(anari_DIR ${VISIT_ANARI_DIR}/lib/cmake/anari-${ANARI_VERSION}
            CACHE PATH 
            "The directory containing the ANARI config files." FORCE)
        MARK_AS_ADVANCED(anari_DIR)
    ENDIF(NOT DEFINED anari_DIR)

    #Find ANARI
    set(CMAKE_LIBRARY_PATH ${VISIT_ANARI_DIR}/lib ${CMAKE_LIBRARY_PATH})
    FIND_PACKAGE(anari)
ENDIF()

IF(ANARI_FOUND)
    ADD_DEFINITIONS(-DVISIT_ANARI)

    # Include directories
    GET_TARGET_PROPERTY(_INCLUDE_DIRS anari::anari INTERFACE_INCLUDE_DIRECTORIES)
    SET(ANARI_INCLUDE_DIRS ${_INCLUDE_DIRS} CACHE PATH "ANARI include directories" FORCE)
    MARK_AS_ADVANCED(ANARI_INCLUDE_DIRS)    
    
    # Install Headers
    IF(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
      INSTALL(DIRECTORY ${VISIT_ANARI_DIR}/include/anari
        DESTINATION
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                         GROUP_WRITE GROUP_READ
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                              GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                          WORLD_READ WORLD_EXECUTE
        PATTERN ".git" EXCLUDE)
    ENDIF()

    # Just the .so libs. If the .so is a symlink to the full version 
    # the install library logic will correctly install both the full 
    # version and the .so symlink, so only the .so is needed to be 
    # sent to the function.
    FILE(GLOB ANARI_LIBRARIES ${VISIT_ANARI_DIR}/lib/lib*.so)

    # Install libs
    FOREACH(l ${ANARI_LIBRARIES})
      GET_FILENAME_COMPONENT(_name_ ${l} NAME_WE)
      THIRD_PARTY_INSTALL_LIBRARY(${l})
    ENDFOREACH()
    
    #================== ANARI Back-end Libraries
    
    # SET(DLOPEN_LIBS)
    # TODO: Debug libs

    # Example back-end device
    FILE(TO_CMAKE_PATH "$ENV{ANARI_Example_DIR}" _Example_DIR)
    FIND_LIBRARY(ANARI_Example_LIBRARY
     	  NAMES anari_library_example
        PATHS 
          ${VISIT_ANARI_DIR}/lib
          ${_Example_DIR}/lib
    	  DOC "ANARI Example back-end library")

    MARK_AS_ADVANCED(ANARI_Example_LIBRARY)

    IF(ANARI_Example_LIBRARY)
      # LIST(APPEND DLOPEN_LIBS ${ANARI_Example_LIBRARY})
      ADD_DEFINITIONS(-DHAVE_ANARI_EXAMPLE)
      message(STATUS "ANARI Example back-end library found.")
    ENDIF()
      
    # VISRTX back-end device
    FILE(TO_CMAKE_PATH "$ENV{ANARI_VISRTX_DIR}" _VISRTX_DIR)
    FIND_LIBRARY(ANARI_VISRTX_LIBRARY
     	  NAMES anari_library_visrtx 
        PATHS 
          ${VISIT_ANARI_DIR}/lib
          ${_VISRTX_DIR}/lib
    	  DOC "ANARI VISRTX back-end library")

    MARK_AS_ADVANCED(ANARI_VISRTX_LIBRARY)

    IF(ANARI_VISRTX_LIBRARY)
      # LIST(APPEND DLOPEN_LIBS ${ANARI_VISRTX_LIBRARY})
      ADD_DEFINITIONS(-DHAVE_ANARI_VISRTX)
      message(STATUS "ANARI VISRTX back-end library found.")
    ENDIF()

    # USD back-end device
    FILE(TO_CMAKE_PATH "$ENV{ANARI_USD_DIR}" _USD_DIR)
    FIND_LIBRARY(ANARI_USD_LIBRARY
     	  NAMES anari_library_usd 
        PATHS 
          ${VISIT_ANARI_DIR}/lib
          ${_USD_DIR}/lib
    	  DOC "ANARI USD back-end library")

    MARK_AS_ADVANCED(ANARI_USD_LIBRARY)

    IF(ANARI_USD_LIBRARY)
      ADD_DEFINITIONS(-DHAVE_ANARI_USD)
      message(STATUS "ANARI USD back-end library found.")
    ENDIF()

    # ANARI tries to dlopen the back-end libs at runtime
    # so we need ot make sure those libs exist in
    # ${VISIT_BINARY_DIR}/lib/
    # so developer builds can load them
    # IF( NOT WIN32 )
    #     FOREACH(anari_lib ${DLOPEN_LIBS})            
    #       execute_process(COMMAND ${CMAKE_COMMAND} -E copy
    #                               ${anari_lib}
    #                               ${VISIT_BINARY_DIR}/lib/)
    #     ENDFOREACH()
    # ENDIF()

    # MESSAGE(STATUS "ANARI_LIBRARIES: " ${ANARI_LIBRARIES})
ENDIF(ANARI_FOUND)
