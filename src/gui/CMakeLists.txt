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
   monkey-xml
   wxWidgets::wxWidgets
)

target_include_directories(monkey-moore-gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

add_custom_command(TARGET monkey-moore-gui POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
   ${CMAKE_SOURCE_DIR}/assets
   $<TARGET_FILE_DIR:monkey-moore-gui>
)