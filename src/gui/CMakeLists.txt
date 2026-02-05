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

add_executable(monkey-moore-gui ${GUI_SRCS})

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