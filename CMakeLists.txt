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

set(STK_SOURCE_PATH_DEFAULT /usr/local/include/stk)
set(STK_SOURCE_PATH ${STK_SOURCE_PATH_DEFAULT} CACHE PATH "STK src path")

set(FLEXT_SRC_PATH_DEFAULT /usr/local/include)
set(FLEXT_SRC_PATH ${FLEXT_SRC_PATH_DEFAULT} CACHE PATH "Flext src path")

set(FLEXT_INSTALL_PATH_DEFAULT /usr/local/lib)
set(FLEXT_INSTALL_PATH ${FLEXT_INSTALL_PATH_DEFAULT} CACHE PATH "Flext install path")

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
    source/flstk.cpp
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
    source/flsupport.h)
add_library(objects OBJECT
    ${FLEXT_SOURCES}
)

target_compile_definitions(objects PRIVATE FLEXT_USE_CMEM)
target_compile_definitions(objects PRIVATE FLEXT_USE_SIMD)
target_compile_definitions(objects PRIVATE FLEXT_SHARED)
target_compile_definitions(objects PRIVATE FLEXT_EXPORTS)
target_compile_definitions(objects PRIVATE FLEXT_SYS=2)
target_compile_definitions(objects PRIVATE PD)
target_compile_options(objects PRIVATE -O3 -ffast-math -fPIC -fvisibility-inlines-hidden)

target_include_directories(objects PRIVATE ${PD_SOURCE_PATH})
target_include_directories(objects PRIVATE ${STK_SOURCE_PATH})
target_include_directories(objects PRIVATE /usr/local/include/sndobj)

find_package(Threads REQUIRED)
target_link_libraries(objects PRIVATE Threads::Threads)

add_library(${PROJECT_NAME}-pd SHARED $<TARGET_OBJECTS:objects>)
target_compile_options(${PROJECT_NAME}-pd PRIVATE -rdynamic -shared -fPIC -Wl,-rpath,"\$ORIGIN",--enable-new-dtags)
target_link_libraries(${PROJECT_NAME}-pd
    PRIVATE -lc
    PRIVATE -lm
)

install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}-pd.so DESTINATION ${FLEXT_INSTALL_PATH}/${PROJECT_NAME})
install(FILES ${FLEXT_SOURCES} ${FLEXT_HEADERS} DESTINATION ${FLEXT_SRC_PATH}/${PROJECT_NAME})