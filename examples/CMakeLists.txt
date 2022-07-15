####################################################################################################
# SDL2 output sample
####################################################################################################
find_package(SDL2)

if(SDL2_FOUND)
    add_executable(output-sdl2
        output-sdl/main.cpp
    )

    target_include_directories(output-sdl2 PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(output-sdl2 PUBLIC shittygui::shittygui ${SDL2_LIBRARIES})
endif()
