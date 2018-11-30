# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindGTK
# -------
#
# try to find GTK (and glib) and GTKGLArea
#
# ::
#
#   GTK_INCLUDE_DIR   - Directories to include to use GTK
#   GTK_LIBRARIES     - Files to link against to use GTK
#   GTK_FOUND         - GTK was found
#   GTK_GL_FOUND      - GTK's GL features were found

# don't even bother under WIN32
if(UNIX)

  find_path( GTK_gtk_INCLUDE_PATH NAMES gtk/gtk.h
    PATH_SUFFIXES gtk-3.0 gtk30
    PATHS
    /usr/include
    /usr/local/include
    /opt/gnome/include
  )

  find_path( GTK_glib_INCLUDE_PATH NAMES glib.h
    PATH_SUFFIXES gtk-3.0 glib-2.0 glib20
    PATHS
    /usr/include
    /usr/local/include
    /opt/gnome/include
  )

  find_path( GTK_gtkgl_INCLUDE_PATH NAMES gtk/gtkglarea.h
    PATH_SUFFIXES gtk-3.0 gtk30
    PATHS 
    /usr/include
    /usr/local/include
    /opt/gnome/include
  )

  find_library( GTK_gtk_LIBRARY
    NAMES  gtk-3 gtk-3.0
    PATHS 
    /usr/lib
    /usr/local/lib
    /opt/gnome/lib
  )

  find_library( GTK_gdk_LIBRARY
    NAMES  gdk-3 gdk-3.0
    PATHS  
    /usr/lib
    /usr/local/lib
    /opt/gnome/lib
  )

  find_library( GTK_gmodule_LIBRARY
    NAMES  gmodule-2.0 gmodule-export-2.0
    PATHS  
    /usr/lib
    /usr/local/lib
    /opt/gnome/lib
  )

  find_library( GTK_glib_LIBRARY
    NAMES  glib-2.0 glib20
    PATHS  
    /usr/lib
    /usr/local/lib
    /opt/gnome/lib
  )

  find_library( GTK_Xi_LIBRARY
    NAMES Xi
    PATHS 
    /usr/lib
    /usr/local/lib
    /opt/gnome/lib
    )

  find_library( GTK_gthread_LIBRARY
    NAMES  gthread-2.0 gthread20
    PATHS  
    /usr/lib
    /usr/local/lib
    /opt/gnome/lib
  )

  if(GTK_gtk_INCLUDE_PATH
     AND GTK_glib_INCLUDE_PATH
     AND GTK_gtk_LIBRARY
     AND GTK_glib_LIBRARY)

    # Assume that if gtk and glib were found, the other
    # supporting libraries have also been found.

    set( GTK_FOUND "YES" )
    set( GTK_INCLUDE_DIR  ${GTK_gtk_INCLUDE_PATH}
                           ${GTK_glib_INCLUDE_PATH} )
    set( GTK_LIBRARIES  ${GTK_gtk_LIBRARY}
                        ${GTK_gdk_LIBRARY}
                        ${GTK_glib_LIBRARY} )
    message(STATUS "-- Found GTK: ${GTK_gtk_LIBRARY}")
    
    if(GTK_gmodule_LIBRARY)
      set(GTK_LIBRARIES ${GTK_LIBRARIES} ${GTK_gmodule_LIBRARY})
    endif()
    if(GTK_gthread_LIBRARY)
      set(GTK_LIBRARIES ${GTK_LIBRARIES} ${GTK_gthread_LIBRARY})
    endif()
    if(GTK_Xi_LIBRARY)
      set(GTK_LIBRARIES ${GTK_LIBRARIES} ${GTK_Xi_LIBRARY})
    endif()

    if(GTK_gtkgl_INCLUDE_PATH AND GTK_gtkgl_LIBRARY)
      set( GTK_GL_FOUND "YES" )
      set( GTK_INCLUDE_DIR  ${GTK_INCLUDE_DIR}
                            ${GTK_gtkgl_INCLUDE_PATH} )
      set( GTK_LIBRARIES  ${GTK_gtkgl_LIBRARY} ${GTK_LIBRARIES} )
      mark_as_advanced(
        GTK_gtkgl_LIBRARY
        GTK_gtkgl_INCLUDE_PATH
        )
    endif()

  endif()

  mark_as_advanced(
    GTK_gdk_LIBRARY
    GTK_glib_INCLUDE_PATH
    GTK_glib_LIBRARY
    GTK_glibconfig_INCLUDE_PATH
    GTK_gmodule_LIBRARY
    GTK_gthread_LIBRARY
    GTK_Xi_LIBRARY
    GTK_gtk_INCLUDE_PATH
    GTK_gtk_LIBRARY
    GTK_gtkgl_INCLUDE_PATH
    GTK_gtkgl_LIBRARY
  )

endif()



