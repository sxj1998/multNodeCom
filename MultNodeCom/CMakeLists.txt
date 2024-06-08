# Get all the .c files in the current directory
file(GLOB MULT_NODE_COM_SOURCES "*.c")
file(GLOB THREAD_PART "3rd_part/*.c")
file(GLOB BUS_DEVICE "bus_device/*.c")
file(GLOB BUS_DRIVER "bus_driver/*.c")
file(GLOB BUS_CORE "bus_core/*.c")
file(GLOB UTILS "utils/*.c")

file(GLOB_RECURSE INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/")

set(INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd_part"
    "${CMAKE_CURRENT_SOURCE_DIR}/bus_device"
    "${CMAKE_CURRENT_SOURCE_DIR}/bus_driver"
    "${CMAKE_CURRENT_SOURCE_DIR}/bus_core"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils"
)

# Add the communication_code.c file and create a library
add_library(MultNodeCom ${MULT_NODE_COM_SOURCES} 
                        ${THREAD_PART}
                        ${BUS_DEVICE}
                        ${BUS_DRIVER}
                        ${BUS_CORE}
                        ${UTILS} 
)

# Specify the include directories
# target_include_directories(communicate PUBLIC "${INCLUDE_DIRS}")
target_include_directories(MultNodeCom  PUBLIC ${INCLUDE_DIRS})