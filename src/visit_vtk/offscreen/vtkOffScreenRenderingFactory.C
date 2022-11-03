// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkOffScreenRenderingFactory.h"
#include <visit-config.h>
#if LIB_VERSION_LE(VTK, 8,1,0)
#if defined(HAVE_OSMESA)
  #include <vtkOSOpenGLRenderWindow.h>
  VTK_CREATE_CREATE_FUNCTION(vtkOSOpenGLRenderWindow);
#endif
#endif



// ****************************************************************************
//  Method:  vtkOffScreenRenderingFactory Constructor
//
//  Purpose:
//
//  Arguments:
//
//  Modifications:
//
// ****************************************************************************

vtkOffScreenRenderingFactory::vtkOffScreenRenderingFactory()
{
    // If VTK was compiled with VTK_OPENGL_HAS_OSMESA then
    // vtkOSOpenGLRenderWindow was compiled into vtkRenderingOpenGL.
    //
    // If VTK was compiled with VTK_OPENGL_HAS_EGL then
    // vtkEGLRenderWindow was compiled into vtkRenderingOpenGL.
    //
    // If VTK was comiled with VTK_USE_X, then, despite either of the above
    // vtkXOpenGLRenderWindow will be the override when instantiating a
    // vtkRenderWindow.
    //
    // This will cause problems for us on headless nodes, so here we create
    // overrides for vtkXOpenGLRenderWindow, depending on whether we want
    // OSMesa or EGL.
    //

#if LIB_VERSION_LE(VTK,8,1,0)
#if defined(HAVE_OSMESA)
    this->RegisterOverride("vtkXOpenGLRenderWindow",
                           "vtkOSOpenGLRenderWindow",
                           "Render Window Hijack Overrride",
                           1,
                           vtkObjectFactoryCreatevtkOSOpenGLRenderWindow);
#endif
#endif
}



// ****************************************************************************
//  Method:  vtkOffScreenRenderingFactory::ForceOffScreen
//
//  Purpose:
//
//  Arguments:
//
// ****************************************************************************

void
vtkOffScreenRenderingFactory::ForceOffScreen()
{
#if defined(HAVE_OSMESA) 
    vtkOffScreenRenderingFactory *os_factory = vtkOffScreenRenderingFactory::New();
    cerr << "  VTK SourceVersion: " << os_factory->GetVTKSourceVersion() << endl;
    vtkObjectFactory::RegisterFactory(os_factory);
    os_factory->Delete();
#endif
}

