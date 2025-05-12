cmake_minimum_required(VERSION 3.20)
project(enzotechdevicelogin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use MFC in a Static Library
set(CMAKE_MFC_FLAG 2)

# Enable Unicode
add_definitions(-D_UNICODE -DUNICODE -DWIN32 -D_WINDOWS)

# Precompiled header
set(PCH_HEADER "pch.h")
set(PCH_SOURCE "pch.cpp")

# Resource file
set(RES_FILE enzotechdevicelogin.rc)

# Source files
set(SOURCES
    CCustomEdit.cpp
    CLoginDialog.cpp
    CLogoButton.cpp
    CTaskProgressDlg.cpp
    CCustomClock.cpp
    Communicator.cpp
    CScalableBitmapStatic.cpp
    enzo-tech-device-login.cpp
    enzo-tech-device-loginDlg.cpp
    ${PCH_SOURCE}
    utils.cpp
    Uuid.cpp
    ${RES_FILE}
)

# Header files
set(HEADERS
    CCustomEdit.h
    CLoginDialog.h
    CLogoButton.h
    CTaskProgressDlg.h
    CCustomClock.h
    Communicator.h
    CScalableBitmapStatic.h
    Definitions.h
    enzo-tech-device-login.h
    enzo-tech-device-loginDlg.h
    framework.h
    ${PCH_HEADER}
    Resource.h
    Settings.h
    targetver.h
    utils.h
    Uuid.h
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/AsyncMFCDialog/include
)

# Add executable with WIN32 to avoid console window
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# Precompiled header support (MSVC only)
if(MSVC)
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH_HEADER})

    # Compiler flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/D_DEBUG /MDd>
        $<$<CONFIG:Release>:/DNDEBUG /MD /GL /O2>
    )

    # Linker flags (UAC settings for Release only)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY
            LINK_FLAGS " /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
        )
    endif()
endif()

# Copy resources after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/res/TimeKeeping.ico"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/settings.ini"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)