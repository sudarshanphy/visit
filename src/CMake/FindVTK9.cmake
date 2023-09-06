# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Notes: Copied from FindVisItVTK.cmake and reworked for VTK-9.
#
# Modifications:
#  Kathleen Biagas, Jun 29, 2023
#  WrappingPythonCore needs special logic, has Python version as part of name.
#
#*****************************************************************************

# Use the VTK_DIR hint from the config-site .cmake file

if(EXISTS ${VISIT_VTK_DIR}/lib/cmake/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}/VTKConfig.cmake)
    SET(VTK_DIR ${VISIT_VTK_DIR}/lib/cmake/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION})
endif()

message(STATUS "Checking for VTK in ${VTK_DIR}")

# Set up our list of required and optional vtk modules
set(REQ_VTK_MODS
        CommonCore
        CommonDataModel
        FiltersCore
        FiltersFlowPaths
        FiltersHybrid
        FiltersModeling
        IOGeometry
        IOLegacy
        IOPLY
        IOXML
        IOXMLParser
        InteractionStyle
        RenderingAnnotation
        RenderingOpenGL2
        RenderingVolume
        RenderingVolumeOpenGL2
        glew)

if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    list(APPEND REQ_VTK_MODS GUISupportQt)
    set(Qt5_DIR ${VISIT_QT_DIR}/lib/cmake/Qt5)
endif()

if(OSPRAY_FOUND)
    list(APPEND REQ_VTK_MODS RenderingRayTracing)
endif()

# Optional
set(OPT_VTK_MODS
       GeovisCore # Cartographic Projection Operator
       IOEnSight  # EnSight Database
       libxml2    # Xdmf Database
   )


find_package(VTK ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}
    REQUIRED ${REQ_VTK_MODS} ${VTK_QT_MODS}
    OPTIONAL_COMPONENTS ${OPT_VTK_MODS}
    NO_MODULE
    PATHS ${VTK_DIR})

if(NOT ${VTK_FOUND})
    message(FATAL_ERROR "VTK is required to build VisIt.")
    return()
endif()

# Ensure we have all the required modules:
foreach(module ${REQ_VTK_MODS} ${VTK_QT_MODS})
    if(NOT TARGET VTK::${module})
        message(ERROR " VisIt requires VTK::${module}")
    endif()
endforeach()

message(STATUS "  VTK_FOUND=${VTK_FOUND}")
message(STATUS "  VTK_MAJOR_VERSION=${VTK_MAJOR_VERSION}")
message(STATUS "  VTK_MINOR_VERSION=${VTK_MINOR_VERSION}")
message(STATUS "  VTK_BUILD_VERSION=${VTK_BUILD_VERSION}")
message(STATUS "  VTK_PREFIX_PATH=${VTK_PREFIX_PATH}")
message(STATUS "  VTK_PYTHON_VERSION=${VTK_PYTHON_VERSION}")
message(STATUS "  VTK_PYTHONPATH=${VTK_PYTHONPATH}")
message(STATUS "  VTK_LIBRARIES=${VTK_LIBRARIES}")
message(STATUS "  VTK_AVAILABLE_COMPONENTS=${VTK_AVAILABLE_COMPONENTS}")


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
else()
    if(NOT WIN32)
        set(pathnameandprefix "${VTK_PREFIX_PATH}/lib64/lib")
    else()
        SET(pathnameandprefix "${VTK_PREFIX_PATH}/bin/")
        SET(pathnameandprefixlib "${VTK_PREFIX_PATH}/lib/")
    endif()
    macro(SETUP_INSTALL vtk_component)
        if(${vtk_component} MATCHES "vtksys")
          set(LIBNAME   ${pathnameandprefix}${vtk_component}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        elseif(${vtk_component} MATCHES "WrappingPythonCore")
          # also needs PYTHON_VERSION
          set(LIBNAME   ${pathnameandprefix}vtk${vtk_component}${PYTHON_VERSION}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        else()
            set(LIBNAME   ${pathnameandprefix}vtk${vtk_component}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        endif()
        if(EXISTS ${LIBNAME})
            THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
        endif()

        if(WIN32)
            # install .lib versions, too
            set(LIBNAME   ${pathnameandprefixlib}vtk${vtk_component}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.lib)
            if(EXISTS ${LIBNAME})
                THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
            endif()
        endif()
    endmacro()

    # Base libs and their python wrappings
# could we just take the vtk 'lib' directory and install it with one command?
# would it handle symlinks properly?
    foreach(VTKLIB ${VTK_AVAILABLE_COMPONENTS})
        SETUP_INSTALL("${VTKLIB}")
    endforeach()

    if(VISIT_HEADERS_SKIP_INSTALL)
        message(STATUS "Skipping vtk headers installation")
    else()
        install(DIRECTORY ${VTK_PREFIX_PATH}/include/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk
            FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                             GROUP_WRITE GROUP_READ
                             WORLD_READ
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                  GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                  WORLD_READ WORLD_EXECUTE)
    endif()
endif()

# check for python wrappers
set(VTK_PY_WRAPPERS_DIR ${VTK_PREFIX_PATH}/${VTK_PYTHONPATH})
message(STATUS "  VTK_PY_WRAPPERS_DIR=${VTK_PY_WRAPPERS_DIR}")

if(EXISTS ${VTK_PY_WRAPPERS_DIR}/vtkmodules)
    message(STATUS "Found VTK Python Wrappers - ${VTK_PY_WRAPPERS_DIR}")

    if(VISIT_VTK_SKIP_INSTALL)
        message(STATUS "Skipping installation of VTK Python bindings")
    else()
        install(FILES ${VTK_PY_WRAPPERS_DIR}/vtk.py
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ)

        install(DIRECTORY ${VTK_PY_WRAPPERS_DIR}/vtkmodules
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
                FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                                 GROUP_WRITE GROUP_READ
                                             WORLD_READ
                DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                      GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                                  WORLD_READ WORLD_EXECUTE)
    endif()

    set(VTK_PYTHON_WRAPPERS_FOUND TRUE)
else()
    set(VTK_PYTHON_WRAPPERS_FOUND FALSE)
endif()

mark_as_advanced(VTK_PYTHON_WRAPPERS_FOUND)

if(WIN32 AND VISIT_MESA_REPLACE_OPENGL AND VISIT_MESAGL_DIR)
    # prepare for drop-in replacement of mesa with opengl, if all pieces are in place
    if(EXISTS ${VISIT_VTK_DIR}/bin/vtkTestOpenGLVersion.exe)
        install(FILES ${VISIT_VTK_DIR}/bin/vtkTestOpenGLVersion.exe
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
    endif()
endif()

unset(VTK_DIR)


# Adds ALIAS named visit_${targ} for every vtk target that is used by visit
# (the REQUIRED and OPTIONAL lists). Also adds the VTK_INCLUDE_DIRS to the
# alias target since the includes aren't added to vtk targets in version 8.
function(add_vtk_alias targ)
    set(short_targ ${targ})
    string(SUBSTRING ${targ} 0 5 starts_with_vtk)
    if(starts_with_vtk STREQUAL "VTK::")
        string(SUBSTRING ${targ} 5 -1 short_targ)
    endif()

    set(vtkTargetName VTK::${short_targ})
message("investigating ${vtkTargetName}")
    if(NOT TARGET ${vtkTargetName})
        return()
    endif()
    # do we have a vtk target like VTK::vtkxxx e.g. VTK::vtksys?
    # we don't want to create an alias named visit_vtkvtksys 
    string(SUBSTRING ${short_targ} 0 3 starts_with_vtk)
    if(starts_with_vtk STREQUAL "vtk") 
        string(SUBSTRING ${short_targ} 3 -1 short_targ)
    endif()
    set(targetName visit_vtk${short_targ})
    if(TARGET ${targetName})
        return()
    endif()
message("creating ${targetName} alias for ${vtkTargetName}")
    add_library(${targetName} ALIAS ${vtkTargetName})
    if(Win32)
        get_target_property(deps ${vtkTargetName} INTERFACE_LINK_LIBRARIES)
    else()
        get_target_property(deps ${vtkTargetName} IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE)
    endif()
    if(deps)
        foreach(dep ${deps})
            add_vtk_alias(${dep})
        endforeach()
    endif()
endfunction()

foreach(targ ${REQ_VTK_MODS} ${OPT_VTK_MODS})
    add_vtk_alias(${targ})
endforeach()

# add variables that capture these targets that may not be built  
if(NOT vtkjpeg_LIBRARIES AND TARGET VTK::jpeg)
    set(vtkjpeg_LIBRARIES VTK::jpeg)
endif()
if(NOT vtkpng_LIBRARIES AND TARGET VTK::png)
    set(vtkpng_LIBRARIES VTK::png)
endif()
if(NOT vtktiff_LIBRARIES AND TARGET VTK::tiff)
    set(vtktiff_LIBRARIES VTK::tiff)
endif()
