cmake_minimum_required(VERSION 3.26)
project(raspi_ndi_transmitter)

set(CMAKE_CXX_STANDARD 23)

# Include paths for NDI SDK
include_directories(/path/to/NDI_SDK/include)
link_directories(/path/to/NDI_SDK/lib)

# Add executable
add_executable(raspi_ndi_transmitter src/main.cpp)
target_link_libraries(raspi_ndi_transmitter PRIVATE ndi)