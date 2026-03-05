include(FetchContent)

FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG 9.0.0
)

# Force TinyXML2 to skip building its tests
set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(tinyxml2)

find_package(wxWidgets REQUIRED COMPONENTS core base adv)

set(GUI_SRCS
   monkey_app.cpp
   monkey_frame.cpp
   monkey_prefs.cpp
)

add_executable(monkey-moore-gui WIN32 ${GUI_SRCS})

target_link_libraries(monkey-moore-gui
   PRIVATE
   monkey-core
   tinyxml2::tinyxml2
   wxWidgets::wxWidgets
)

target_include_directories(monkey-moore-gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET monkey-moore-gui POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
   ${CMAKE_SOURCE_DIR}/assets
   $<TARGET_FILE_DIR:monkey-moore-gui>
)

# On Windows we ship the executable along with all DLLs it depends on
if(WIN32)
    install(TARGETS monkey-moore-gui RUNTIME DESTINATION . COMPONENT MonkeyMooreRelease)

    set(CMAKE_INSTALL_UCRT_LIBRARIES FALSE)
    include(InstallRequiredSystemLibraries)

    # collects the Windows C++ Redistributable DLLs so we can copy them to the staging dir 
    set(MSVC_RUNTIME_DLLS "")
    foreach(lib IN LISTS CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        if(lib MATCHES "vcruntime140\\.dll$" OR lib MATCHES "vcruntime140_1\\.dll$" OR lib MATCHES "msvcp140\\.dll$")
            list(APPEND MSVC_RUNTIME_DLLS "${lib}")
        endif()
    endforeach()
    
    install(PROGRAMS ${MSVC_RUNTIME_DLLS} DESTINATION . COMPONENT MonkeyMooreRelease)
endif()
