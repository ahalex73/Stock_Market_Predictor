set ( PARALLEL_APP_COMMON_SRC
    common/src/main.cpp
)

set (PARALLEL_APP_COMMON_INC
    common/inc/stdafx.h
    common/inc/transport_interface.h
    common/inc/transport_factory.h
)

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
set( PARALLEL_APP_WIN_SRC 
    platform/windows/src/windows_udp_transport.cpp
) 
set( PARALLEL_APP_WIN_INC 
    platform/windows/inc/windows_udp_transport.h
) 
else() 
set( PARALLEL_APP_WIN_SRC )
set( PARALLEL_APP_WIN_INC )
endif()

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
set( PARALLEL_APP_LINUX_SRC 
    
)
set( PARALLEL_APP_LINUX_INC 
    
)
else() 
set( PARALLEL_APP_LINUX_SRC )
set( PARALLEL_APP_LINUX_INC )
endif()

set( _PARALLEL_APP_SOURCEFILES
    ${PARALLEL_APP_COMMON_SRC}
    ${PARALLEL_APP_COMMON_INC}
    ${PARALLEL_APP_WIN_SRC}
    ${PARALLEL_APP_WIN_INC}
    ${PARALLEL_APP_LINUX_SRC}
    ${PARALLEL_APP_LINUX_INC}
    ${PARALLEL_APP_TOPLEVEL}
    files.cmake
)

source_group( "" FILES ${PARALLEL_APP_TOPLEVEL})
source_group( common/src FILES ${PARALLEL_APP_COMMON_SRC})
source_group( common/inc FILES ${PARALLEL_APP_COMMON_INC})
source_group( platform/windows/src FILES ${PARALLEL_APP_WIN_SRC})
source_group( platform/windows/inc FILES ${PARALLEL_APP_WIN_INC})
source_group( platform/linux/src FILES ${PARALLEL_APP_LINUX_SRC})
source_group( platform/linux/inc FILES ${PARALLEL_APP_LINUX_INC})