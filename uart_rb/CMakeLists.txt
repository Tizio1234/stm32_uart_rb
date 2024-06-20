cmake_minimum_required(VERSION 3.22)

project(uart_rb)

add_library(${PROJECT_NAME} STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/uart_rb.c
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc) # Public include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src) # Private include directories

target_link_libraries(${PROJECT_NAME} PUBLIC) # Public dependencies(see at the bottom of this file)
target_link_libraries(${PROJECT_NAME} PRIVATE) # Private dependencies

# For now(june 2024), stm32cubemx generates a single INTERFACE library for both project and driver sources.
# This makes it so that for each target that needs drivers, all of the stm32cubemx sources are recompiled.
# To solve this issue, a static wrapper library for external libraries(like this one) should be used, this is going to be implemented in the example pretty soon
# For the project(the executable), it should still be used the INTERFACE version otherwise there could be unintended consequences(e.g. tick not working)
