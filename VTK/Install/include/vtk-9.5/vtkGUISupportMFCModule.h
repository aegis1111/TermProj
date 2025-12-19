
#ifndef VTKGUISUPPORTMFC_EXPORT_H
#define VTKGUISUPPORTMFC_EXPORT_H

#ifdef VTKGUISUPPORTMFC_STATIC_DEFINE
#  define VTKGUISUPPORTMFC_EXPORT
#  define VTKGUISUPPORTMFC_NO_EXPORT
#else
#  ifndef VTKGUISUPPORTMFC_EXPORT
#    ifdef GUISupportMFC_EXPORTS
        /* We are building this library */
#      define VTKGUISUPPORTMFC_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define VTKGUISUPPORTMFC_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef VTKGUISUPPORTMFC_NO_EXPORT
#    define VTKGUISUPPORTMFC_NO_EXPORT 
#  endif
#endif

#ifndef VTKGUISUPPORTMFC_DEPRECATED
#  define VTKGUISUPPORTMFC_DEPRECATED __declspec(deprecated)
#endif

#ifndef VTKGUISUPPORTMFC_DEPRECATED_EXPORT
#  define VTKGUISUPPORTMFC_DEPRECATED_EXPORT VTKGUISUPPORTMFC_EXPORT VTKGUISUPPORTMFC_DEPRECATED
#endif

#ifndef VTKGUISUPPORTMFC_DEPRECATED_NO_EXPORT
#  define VTKGUISUPPORTMFC_DEPRECATED_NO_EXPORT VTKGUISUPPORTMFC_NO_EXPORT VTKGUISUPPORTMFC_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VTKGUISUPPORTMFC_NO_DEPRECATED
#    define VTKGUISUPPORTMFC_NO_DEPRECATED
#  endif
#endif

/* VTK-HeaderTest-Exclude: vtkGUISupportMFCModule.h */

/* Include ABI Namespace */
#include "vtkABINamespace.h"

#endif /* VTKGUISUPPORTMFC_EXPORT_H */
