
function (disallow_intree_builds)
  # Adapted from LLVM's toplevel CMakeLists.txt file
  if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
    message(FATAL_ERROR "
      In-source builds are not allowed. CMake would overwrite the
      makefiles distributed with utf8proc. Please create a directory
      and run cmake from there. Building in a subdirectory is
      fine, e.g.:
      
        mkdir build
        cd build
        cmake ..
      
      This process created the file `CMakeCache.txt' and the
      directory `CMakeFiles'. Please delete them.
      
      ")
  endif()
endfunction()
