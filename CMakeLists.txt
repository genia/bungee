set(BUNGEE_VERSION 0.0.0 CACHE STRING "Bungee version string")
set(BUNGEE_SELF_TEST 0 CACHE STRING "Enable Bungee self tests (0=off, 1=fast, 2=full)")
option(BUNGEE_BUILD_SHARED_LIBRARY "Build shared, dynamic Bungee library in addition to a static libary" ON)
set(BUNGEE_PRESET "" CACHE STRING "Name of the preset that we're building")

if (BUNGEE_BUILD_SHARED_LIBRARY)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

cmake_minimum_required(VERSION 3.30...3.31)

project(bungee VERSION ${BUNGEE_VERSION})
set(CMAKE_CXX_STANDARD 20)

if (WIN32)
  set(BUNGEE_EXPORT "__declspec(dllexport)")
else()
  set(BUNGEE_EXPORT "__attribute__((visibility(\"default\")))")
endif()

# Bungee library target: "bungee_library"
file(GLOB BUNGEE_LIBRARY_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/bungee/*.h")
file(GLOB BUNGEE_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
if (BUNGEE_BUILD_SHARED_LIBRARY)
  add_library(bungee_library SHARED ${BUNGEE_LIBRARY_HEADERS} ${BUNGEE_SOURCE_FILES})
else()
  add_library(bungee_library STATIC ${BUNGEE_LIBRARY_HEADERS} ${BUNGEE_SOURCE_FILES})
endif()  
target_include_directories(bungee_library PRIVATE  submodules/eigen submodules .)
target_compile_definitions(bungee_library PRIVATE
  BUNGEE_SELF_TEST=${BUNGEE_SELF_TEST}
  eigen_assert=BUNGEE_ASSERT1
  EIGEN_DONT_PARALLELIZE=1
  BUNGEE_VISIBILITY=${BUNGEE_EXPORT}
)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp PROPERTIES COMPILE_DEFINITIONS BUNGEE_VERSION=\"${BUNGEE_VERSION}\")
target_compile_options(bungee_library PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fwrapv>)
target_link_libraries(bungee_library PRIVATE kissfft)
if (ANDROID)
  target_link_libraries(bungee_library PRIVATE log)
endif()
set_target_properties(bungee_library PROPERTIES OUTPUT_NAME bungee)

# Bungee command-line executable target: "bungee_executable"
add_executable(bungee_executable cmd/main.cpp)
target_include_directories(bungee_executable PRIVATE submodules/cxxopts/include .)
target_link_libraries(bungee_executable PRIVATE bungee_library)
set_target_properties(bungee_executable PROPERTIES 
  OUTPUT_NAME bungee
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}"
)

# KissFFT build
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
set(KISSFFT_PKGCONFIG OFF CACHE INTERNAL "" FORCE)
set(KISSFFT_STATIC ON CACHE INTERNAL "" FORCE)
set(KISSFFT_TEST OFF CACHE INTERNAL "" FORCE)
set(KISSFFT_TOOLS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(submodules/kissfft EXCLUDE_FROM_ALL) # -Wcast-align

# Installation
if (BUNGEE_BUILD_SHARED_LIBRARY)
  if (APPLE)
    set_target_properties(bungee_library PROPERTIES
      FRAMEWORK TRUE
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
      MACOSX_FRAMEWORK_BUNDLE_VERSION ${BUNGEE_VERSION}
      MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${BUNGEE_VERSION}
      VERSION ${BUNGEE_VERSION}
    )
    set_source_files_properties(
      ${BUNGEE_LIBRARY_HEADERS} 
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Headers/bungee
    )
  endif()
  install(FILES 
    ${BUNGEE_LIBRARY_HEADERS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bungee
  )
  install(TARGETS bungee_library bungee_executable
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUNGEE_PRESET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUNGEE_PRESET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUNGEE_PRESET}
    BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUNGEE_PRESET}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUNGEE_PRESET}
  )
  install(FILES 
    cmd/main.cpp  
    submodules/cxxopts/include/cxxopts.hpp
    DESTINATION ${CMAKE_INSTALL_PREFIX}/sample
  )
endif()
