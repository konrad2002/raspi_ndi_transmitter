cmake_minimum_required(VERSION 3.25)
project(raspi_ndi_transmitter)

set(CMAKE_CXX_STANDARD 23)

# Include paths for NDI SDK
include_directories(./NDI_SDK/include)
link_directories(./NDI_SDK/lib)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Add executable
add_executable(raspi_ndi_transmitter src/main.cpp)
# Link libraries for NDI and OpenCV
target_link_libraries(raspi_ndi_transmitter PRIVATE ndi ${OpenCV_LIBS})
