cmake_minimum_required(VERSION 3.26)
project(flext VERSION 0.6.2 LANGUAGES C CXX)

# setting Pd source path
if(UNIX AND NOT APPLE)
    set(PD_SOURCE_PATH_DEFAULT /usr/local/pd/src)
elseif(UNIX AND APPLE)
    set(PD_SOURCE_PATH_DEFAULT /Applications/Pd-0.54-0.app/Contents/Resources/src)
elseif(WIN32)
    set(PD_SOURCE_PATH_DEFAULT c:/pd/src)
endif()

set(PD_SOURCE_PATH ${PD_SOURCE_PATH_DEFAULT} CACHE PATH "Pd src path")
set(PD_INSTANCE OFF CACHE BOOL "Builds with PDINSTANCE defined")

# setting stk source
# set(STK_SOURCE_PATH_DEFAULT /usr/local/include/stk)
# set(STK_SOURCE_PATH ${STK_SOURCE_PATH_DEFAULT} CACHE PATH "STK src path")

# setting flext install src
set(FLEXT_SRC_PATH_DEFAULT /usr/local/include)
set(FLEXT_SRC_PATH ${FLEXT_SRC_PATH_DEFAULT} CACHE PATH "Flext src path")

# setting flext install path
set(FLEXT_INSTALL_PATH_DEFAULT /usr/local/lib)
set(FLEXT_INSTALL_PATH ${FLEXT_INSTALL_PATH_DEFAULT} CACHE PATH "Flext install path")

set(FLEXT_PREFIX_LIB ${PROJECT_NAME}-pd CACHE STRING "Flext lib name")

set(FLEXT_SOURCES
    source/flatom.cpp
    source/flatom_part.cpp
    source/flatom_pr.cpp
    source/flattr.cpp
    source/flattr_ed.cpp
    source/flbase.cpp
    source/flbind.cpp
    source/flbuf.cpp
    source/fldsp.cpp
    source/flext.cpp
    source/flitem.cpp
    source/fllib.cpp
    source/flmap.cpp
    source/flmeth.cpp
    source/flmsg.cpp
    source/flout.cpp
    source/flproxy.cpp
    source/flqueue.cpp
    source/flsimd.cpp
    source/flsupport.cpp
    source/flthr.cpp
    source/fltimer.cpp
    source/flutil.cpp
    source/flxlet.cpp)

set(FLEXT_HEADERS
    source/flbase.h
    source/flclass.h
    source/flcontainers.h
    source/fldefs_attradd.h
    source/fldefs_attrcb.h
    source/fldefs_attrvar.h
    source/fldefs.h
    source/fldefs_hdr.h
    source/fldefs_methadd.h
    source/fldefs_methbind.h
    source/fldefs_methcall.h
    source/fldefs_methcb.h
    source/fldefs_meththr.h
    source/fldefs_setup.h
    source/fldoxygen.h
    source/fldsp.h
    source/flext.h
    source/flfeatures.h
    source/flinternal.h
    source/flmap.h
    source/flmspbuffer.h
    source/flpopns.h
    source/flprefix.h
    source/flpushns.h
    source/flsndobj.h
    source/flstdc.h
    source/flstk.h
    source/flsupport.h
)

set(FLEXT_LOCKFREE
    source/lockfree/atomic_int.hpp
    source/lockfree/atomic_ptr.hpp
    source/lockfree/branch_hints.hpp
    source/lockfree/cas.hpp
    source/lockfree/fifo.hpp
    source/lockfree/prefix.hpp
    source/lockfree/stack.hpp)

# ###########        OBJECTS RULE        ############
add_library(objects OBJECT
    ${FLEXT_SOURCES}
)

target_compile_definitions(objects PRIVATE FLEXT_USE_CMEM)
target_compile_definitions(objects PRIVATE FLEXT_USE_SIMD)
target_compile_definitions(objects PRIVATE FLEXT_EXPORTS)
target_compile_definitions(objects PRIVATE FLEXT_SHARED)
target_compile_definitions(objects PRIVATE FLEXT_SYS=2)
target_compile_definitions(objects PRIVATE PD)

if(${PD_INSTANCE})
    target_compile_definitions(objects PRIVATE PDINSTANCE)
    target_compile_definitions(objects PRIVATE PDTHREAD)
endif()

target_compile_options(objects PRIVATE -O3 -ffast-math -fPIC -fvisibility-inlines-hidden)

target_include_directories(objects PRIVATE ${PD_SOURCE_PATH})
target_include_directories(objects PRIVATE ${STK_SOURCE_PATH})
target_include_directories(objects PRIVATE /usr/local/include/sndobj)

find_package(Threads REQUIRED)
target_link_libraries(objects PRIVATE Threads::Threads)


# ###########        SINGLE RULE        ############
set(FLEXT_SINGLE_LIB "${FLEXT_PREFIX_LIB}_s")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(FLEXT_SINGLE_LIB "${FLEXT_SINGLE_LIB}d")
endif()

if(PD_INSTANCE)
    set(FLEXT_SINGLE_LIB "${FLEXT_SINGLE_LIB}_i")
endif()

set(FLEXT_SINGLE_LIB ${FLEXT_SINGLE_LIB} CACHE STRING "Flext multi library name")

add_library(${FLEXT_SINGLE_LIB} STATIC $<TARGET_OBJECTS:objects>)
target_compile_definitions(${FLEXT_SINGLE_LIB} PRIVATE FLEXT_THREADS)

set( LEGACY_LINK   lib${FLEXT_SINGLE_LIB}.a)
set( LEGACY_TARGET lib${FLEXT_SINGLE_LIB}.a.${PROJECT_VERSION})
install(FILES ${CMAKE_BINARY_DIR}/${LEGACY_LINK} DESTINATION ${FLEXT_INSTALL_PATH})


############        MULTI  RULE        ############
set(FLEXT_MULTI_LIB "${FLEXT_PREFIX_LIB}_t")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(FLEXT_MULTI_LIB "${FLEXT_MULTI_LIB}d")
endif()

if(PD_INSTANCE)
    set(FLEXT_MULTI_LIB "${FLEXT_MULTI_LIB}_i")
endif()

set(FLEXT_MULTI_LIB ${FLEXT_MULTI_LIB} CACHE STRING "Flext multi library name")

add_library(${FLEXT_MULTI_LIB} STATIC $<TARGET_OBJECTS:objects>)
target_compile_definitions(${FLEXT_MULTI_LIB} PRIVATE FLEXT_THREADS)

set( LEGACY_LINK   lib${FLEXT_MULTI_LIB}.a)
set( LEGACY_TARGET lib${FLEXT_MULTI_LIB}.a.${PROJECT_VERSION})
install(FILES ${CMAKE_BINARY_DIR}/${LEGACY_LINK} DESTINATION ${FLEXT_INSTALL_PATH})


# ###########        SHARED RULE        ############
set(FLEXT_SHARED_LIB "${FLEXT_PREFIX_LIB}")
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(FLEXT_SHARED_LIB "${FLEXT_SHARED_LIB}_d")
endif()

if(PD_INSTANCE)
    set(FLEXT_SHARED_LIB "${FLEXT_SHARED_LIB}_i")
endif()

set(FLEXT_SHARED_LIB ${FLEXT_SHARED_LIB} CACHE STRING "Flext shared library name")

add_library(${FLEXT_SHARED_LIB} SHARED $<TARGET_OBJECTS:objects>)
target_compile_options(${FLEXT_SHARED_LIB} PRIVATE -rdynamic -shared -fPIC -Wl,-rpath,"\$ORIGIN",--enable-new-dtags)

target_compile_definitions(${FLEXT_SHARED_LIB} PRIVATE FLEXT_SHARED)
target_link_libraries(${FLEXT_SHARED_LIB}
    PRIVATE -lc
    PRIVATE -lm
)

set(FLEXT_LIB_FINAL "lib${FLEXT_PREFIX_LIB}")
if(NOT PD_INSTANCE)
set(FLEXT_LIB_FINAL "${FLEXT_LIB_FINAL}.so")
else()
set(FLEXT_LIB_FINAL "${FLEXT_LIB_FINAL}_i.so")
endif()

install(FILES ${CMAKE_BINARY_DIR}/${FLEXT_LIB_FINAL} DESTINATION ${FLEXT_INSTALL_PATH})

install(FILES ${FLEXT_SOURCES} ${FLEXT_HEADERS} DESTINATION ${FLEXT_SRC_PATH}/${PROJECT_NAME})
install(FILES ${FLEXT_LOCKFREE} DESTINATION ${FLEXT_SRC_PATH}/${PROJECT_NAME}/lockfree)