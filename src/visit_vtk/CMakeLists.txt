# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

project(visit_vtk)

add_subdirectory(lightweight)
add_subdirectory(full)

# if osmesa or egl is enabled, add our offscreen rendering factory
if(HAVE_OSMESA AND VTK_VERSION VERSION_EQUAL "8.1.0")
    add_subdirectory(offscreen)
endif()

