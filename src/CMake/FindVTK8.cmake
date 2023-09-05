# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Thu Dec 10 17:53:36 MT 2009
#   Use the same find routines whether on Windows or not.
#
#   Cyrus Harrison, Fri Feb 19 15:41:04 PST 2010
#   Added install of vtk python wrappers (if they exist).
#
#   Cyrus Harrison, Tue Mar  9 07:51:00 PST 2010
#   Added install_name_tool patch of of vtk python wrappers (if they exist)
#
#   Kathleen Bonnell,  Wed Mar 17 10:03:52 MST 2010
#   Prevent '.svn' from being included when installing directories.
#   Change how python wrappers are handled on windows, due to different
#   VTK directory structure.
#
#   Kathleen Bonnell,  Thu Dec 2 15:30:05 MST 2010
#   Add .lib files for Install on Windows.
#
#   Tom Fogal, Fri Jan 28 13:35:29 MST 2011
#   Account for new VTK installation structure.
#
#   Kathleen Bonnell, Tue Jun  7 11:10:51 PDT 2011
#   Don't add VTK's MangledMesa directory to VTK_INCLUDE_DIRS unless VTK
#   was built with MangledMesa.
#
#   Brad Whitlock, Mon Nov 21 10:22:56 PST 2011
#   Print out some variables that were supposed to have been set by the
#   FindVTK.cmake routine so we know better what's going on. Also deal with
#   the case where VTK has been installed normally (with vtk-5.8 subdirs)
#   and change how Python filters are located on Windows.
#
#   Cyrus Harrison, Tue Sep 25 12:09:39 PDT 2012
#   Added Geoviz libs
#
#   Kathleen Biagas, Thu Dec 6 10:26:54 PST 2012
#   Use VTK_LIBRARIES and others in determining what to install, rather than
#   listing individually, which is prone to omissions.
#
#   Kathleen Biagas, Wed Jan 30 17:54:21 MST 2013
#   Preliminary changes for VTK-6.
#
#   Kathleen Biagas, Thu Feb 21 16:04:58 MST 2013
#   Set VTK_LIBRARY_DIRS manually.
#
#   Kathleen Biagas, Thu May  2 10:11:44 PDT 2013
#   Don't attempt the python wrappers if we don't have PYTHON_VERSION.
#
#   Kathleen Biagas, Fri May  3 17:00:03 MST 2013
#   Ensure that the vtk install has all the libraries we require.
#
#   Alister Maguire, Thu Sep 14 14:26:07 PDT 2017
#   Added vtkRenderingVolumeOpenGL2
#
#   Kathleen Biagas, Fri Jan  17 2020
#   Install vtkTestOpenGLVersion.exe on Windows when needed.
#
#   Kathleen Biagas, Sat July 17, 2022
#   Renamed from FindVisItVTK.cmake.
#
#****************************************************************************/

# Use the VTK_DIR hint from the config-site .cmake file

if(EXISTS ${VISIT_VTK_DIR}/lib/cmake/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}/VTKConfig.cmake)
    set(VTK_DIR ${VISIT_VTK_DIR}/lib/cmake/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION})
endif()

message(STATUS "Checking for VTK in ${VTK_DIR}")

# Set up our list of required and optional vtk modules
set(REQ_VTK_MODS
        vtkCommonCore
        vtkCommonDataModel
        vtkFiltersCore
        vtkFiltersFlowPaths
        vtkFiltersHybrid
        vtkFiltersModeling
        vtkIOGeometry
        vtkIOLegacy
        vtkIOPLY
        vtkIOXMLParser
        vtkInteractionStyle
        vtkRenderingAnnotation
        vtkRenderingOpenGL2
        vtkRenderingVolume
        vtkRenderingVolumeOpenGL2
        vtkglew)

if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
   set(VTK_QT_MODS vtkGUISupportQtOpenGL)
endif()

set(OPT_VTK_MODS
    # Cartographic Projection Operator
    vtkGeovisCore
    # EnSight Reader
    vtkIOEnSight
    vtkRenderingOSPRay
    # Xdmf Reader
    vtklibxml2)

# We don't list our required modules in the find_package call because it
# does funny things with VTK_INCLUDES, and the OPTIONAL_COMPONENTS arg
# causes an error if anything in the optional list is not found, would be better
# if it issued a warning instead. Perhaps one day it will be fixed, and we can
# use this: find_package(VTK 6.0 REQUIRED ${REQ_VTK_MODS} OPTIONAL_COMPONENTS ${OPT_VTK_MODS} NO_MODULE PATHS ${VTK_DIR})

set(Qt5_DIR ${VISIT_QT_DIR}/lib/cmake/Qt5)
find_package(VTK ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} REQUIRED NO_MODULE PATHS ${VTK_DIR})

if(NOT ${VTK_FOUND})
    message(FATAL_ERROR "VTK is required to build VisIt.")
    return()
endif()

# Ensure we have all the required modules:
foreach(module ${REQ_VTK_MODS})
    if(NOT TARGET ${module})
        message(ERROR "VisIt requires ${module}")
    endif()
endforeach()


# due to cmake policy 0126, created in cmake 3.21
# if a var is already defined, calling set even with FORCE will not overwrite
# the value so we unset it here to make it clear we want to replace the old
# value with something new
unset(VTK_LIBRARY_DIRS)

set(VTK_LIBRARY_DIRS ${VTK_INSTALL_PREFIX}/lib CACHE PATH "Path to vtk libraries" FORCE)
message(STATUS "  VTK_FOUND=${VTK_FOUND}")
message(STATUS "  VTK_MAJOR_VERSION=${VTK_MAJOR_VERSION}")
message(STATUS "  VTK_MINOR_VERSION=${VTK_MINOR_VERSION}")
message(STATUS "  VTK_BUILD_VERSION=${VTK_BUILD_VERSION}")
message(STATUS "  VTK_INCLUDE_DIRS=${VTK_INCLUDE_DIRS}")
message(STATUS "  VTK_DEFINITIONS=${VTK_DEFINITIONS}")
message(STATUS "  VTK_LIBRARIES=${VTK_LIBRARIES}")
message(STATUS "  VTK_INSTALL_PREFIX=${VTK_INSTALL_PREFIX}")
message(STATUS "  VTK_LIBRARY_DIRS=${VTK_LIBRARY_DIRS}")

# Add install commands for all of the VTK libraries. Is there a better way?
if(APPLE)
    set(SO_EXT "dylib")
elseif(WIN32)
    set(SO_EXT "dll")
else()
    set(SO_EXT "so")
endif()

if(VISIT_VTK_SKIP_INSTALL)
    message(STATUS "Skipping installation of VTK libraries")
else(VISIT_VTK_SKIP_INSTALL)
    if(NOT WIN32)
        set(pathnameandprefix "${VTK_INSTALL_PREFIX}/lib/lib")
    else()
        set(pathnameandprefix "${VTK_INSTALL_PREFIX}/bin/")
        set(pathnameandprefixlib "${VTK_INSTALL_PREFIX}/lib/")
    endif(NOT WIN32)
    macro(SETUP_INSTALL vtklib)
        set(LIBNAME   ${pathnameandprefix}${vtklib}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        if(EXISTS ${LIBNAME})
            THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
        endif()

        if(WIN32)
            # install .lib versions, too
            set(LIBNAME   ${pathnameandprefixlib}${vtklib}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.lib)
            if(EXISTS ${LIBNAME})
                THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
            endif()
        endif()
    endmacro()

    # Base libs and their python wrappings
    foreach(VTKLIB ${VTK_LIBRARIES})
        SETUP_INSTALL("${VTKLIB}")
    endforeach()

    # Python
    if(PYTHON_VERSION)
        # different naming convention.
        # Remove '.' from python version for use in vtk python library naming.
        string(REPLACE "." "" PYVER ${PYTHON_VERSION})
        foreach(VTKLIB ${VTK_LIBRARIES})
            SETUP_INSTALL("${VTKLIB}Python${PYVER}D")
        endforeach()
        SETUP_INSTALL(vtkWrappingPython${PYVER}Core)
    endif()

    # Add install targets for VTK headers too -- but just the vtk-5.0 dir.
    # The VTK_INCLUDE_DIRS may contain stuff like /usr/include or the
    # Python directory and we just want VTK here.
    if(VISIT_HEADERS_SKIP_INSTALL)
        message(STATUS "Skipping vtk headers installation")
    else()
        foreach(X ${VTK_INCLUDE_DIRS})
            if(EXISTS ${X}/vtkActor.h)
               THIRD_PARTY_INSTALL_INCLUDE(vtk ${X})
            endif()
        endforeach()
    endif()
endif()

# check for python wrappers
if(NOT WIN32)
    file(GLOB VTK_PY_WRAPPERS_DIR ${VTK_LIBRARY_DIRS}/python*/)
else ()
    file(GLOB VTK_PY_WRAPPERS_DIR ${VISIT_VTK_DIR}/lib/python*)
endif()
message(STATUS "  VTK_PY_WRAPPERS_DIR=${VTK_PY_WRAPPERS_DIR}")

if(EXISTS ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)
    message(STATUS "Found VTK Python Wrappers - ${VTK_PY_WRAPPERS_DIR}")
    file(GLOB VTK_PY_EGG ${VTK_PY_WRAPPERS_DIR}/site-packages/*.egg*)
    file(GLOB VTK_PY_MODULE ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)

    if(VISIT_VTK_SKIP_INSTALL)
        message(STATUS "Skipping installation of VTK Python bindings")
    else()
        install(FILES ${VTK_PY_EGG}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ)

        install(DIRECTORY ${VTK_PY_MODULE}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
                FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                                 GROUP_WRITE GROUP_READ
                                             WORLD_READ
                DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                      GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                      WORLD_READ             WORLD_EXECUTE)
    endif()

    set(VTK_PYTHON_WRAPPERS_FOUND TRUE)
else()
    set(VTK_PYTHON_WRAPPERS_FOUND FALSE)
endif()

mark_as_advanced(VTK_PYTHON_WRAPPERS_FOUND)

# prepare for drop-in replacement of mesa with opengl, if all pieces are in place
if(WIN32 AND VISIT_MESA_REPLACE_OPENGL AND VISIT_MESAGL_DIR)
    if(EXISTS ${VISIT_VTK_DIR}/bin/vtkTestOpenGLVersion.exe)
        install(FILES ${VISIT_VTK_DIR}/bin/vtkTestOpenGLVersion.exe
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
    endif()
endif()


# Adds ALIAS named visit_${targ} for every vtk target that is used by visit
# (the REQUIRED and OPTIONAL lists). Also adds the VTK_INCLUDE_DIRS to the
# alias target since the includes aren't added to vtk targets in version 8.

function(add_vtk_alias targ)
    set(targetName visit_${targ})
    if(TARGET ${targetName})
        return()
    endif()
message("adding alias for ${targ}")
    add_library(${targetName} ALIAS ${targ})
    target_include_directories(${targ} INTERFACE
            "$<BUILD_INTERFACE:${VTK_INCLUDE_DIRS}>"
            $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/vtk/vtk-${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}>)
    if(Win32)
        get_target_property(deps ${targ} INTERFACE_LINK_LIBRARIES)
    else()
        get_target_property(deps ${targ} IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE)
    endif()
    if(deps)
        foreach(dep ${deps})
            string(SUBSTRING ${dep} 0 3 starts_with_vtk)
            if(starts_with_vtk STREQUAL "vtk")
                add_vtk_alias(${dep})
            endif()
        endforeach()
    endif()
endfunction()

# maybe should use VTK_LIBRARIES instead of our lists which may need
# periodic updating when someone introduces a new vtk lib dependency in
# visit  ?
foreach(targ ${REQ_VTK_MODS} ${OPT_VTK_MODS})
    string(SUBSTRING ${targ} 0 3 starts_with_vtk)
    if(starts_with_vtk STREQUAL "vtk")
                add_vtk_alias(${targ})
    endif()
endforeach()

# right now we only list one library as necessary for Qt support,
# knowing VTK/CMake will pull in the rest
# the names are different between VTK8 and VTK9 so create a
# unique alias here to use when vtk qt support is needed
if(VTK_QT_MODS)
    add_library(visit_vtkQtSupport ALIAS vtkGUISupportQtOpenGL)
endif()


if(NOT vtkjpeg_LIBRARIES AND TARGET vtkjpeg)
    set(vtkjpeg_LIBRARIES vtkjpeg)
endif()
if(NOT vtkpng_LIBRARIES AND TARGET vtkpng)
    set(vtkpng_LIBRARIES vtkpng)
endif()
if(NOT vtktiff_LIBRARIES AND TARGET vtktiff)
    set(vtktiff_LIBRARIES vtktiff)
endif()
