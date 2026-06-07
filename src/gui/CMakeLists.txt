find_package(tinyxml2 CONFIG REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base adv)

set(GUI_HEADERS
   byteswap.hpp
   constants.hpp
   drop_target.hpp
   filesystem_utils.hpp
   dialogs/about_dialog.hpp
   monkey_app.hpp
   monkey_error.hpp
   monkey_frame.hpp
   monkey_options.hpp
   monkey_prefs.hpp
   dialogs/custom_sequences.hpp
   monkey_table.hpp
   monkey_thread.hpp
)

set(GUI_SRCS
   monkey_app.cpp
   monkey_frame.cpp
   monkey_prefs.cpp
   dialogs/about_dialog.cpp
   drop_target.cpp
)

add_executable(monkey-moore-gui WIN32 ${GUI_SRCS} ${GUI_HEADERS})

set_target_properties(monkey-moore-gui PROPERTIES OUTPUT_NAME "monkey-moore")

if(WIN32)
    set_source_files_properties(${CMAKE_SOURCE_DIR}/assets/platform/windows/monkey_res.rc
        PROPERTIES COMPILE_FLAGS  "/I ${CMAKE_SOURCE_DIR}/assets/icons"
    )

    target_sources(monkey-moore-gui PRIVATE 
        ${CMAKE_SOURCE_DIR}/assets/platform/windows/monkey_res.rc
    )
endif()

target_link_libraries(monkey-moore-gui
   PRIVATE
   monkey-core
   tinyxml2::tinyxml2
   ${wxWidgets_LIBRARIES}
)

target_include_directories(monkey-moore-gui PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${wxWidgets_INCLUDE_DIRS}
)

target_compile_definitions(monkey-moore-gui PRIVATE
    ${wxWidgets_DEFINITIONS}
)

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

if(UNIX AND NOT APPLE)
    install(TARGETS monkey-moore-gui RUNTIME DESTINATION bin)

    install(DIRECTORY
            ${CMAKE_SOURCE_DIR}/assets/ui
            ${CMAKE_SOURCE_DIR}/assets/i18n/locales
            DESTINATION share/monkey-moore
    )

    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/16x16.png
            DESTINATION share/icons/hicolor/16x16/apps
            RENAME "monkey-moore.png"
    )

    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/24x24.png
            DESTINATION share/icons/hicolor/24x24/apps
            RENAME "monkey-moore.png"
    )

    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/32x32.png
            DESTINATION share/icons/hicolor/32x32/apps
            RENAME "monkey-moore.png"
    )

    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/48x48.png
            DESTINATION share/icons/hicolor/48x48/apps
            RENAME "monkey-moore.png"
    )

    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/monkey-moore.png
            DESTINATION share/icons/hicolor/512x512/apps
            RENAME "monkey-moore.png"
    )
endif()