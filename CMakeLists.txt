# ==============================================================================
# CMakeLists.txt for lottery3D
# ==============================================================================
cmake_minimum_required(VERSION 3.16)
project(lottery3D VERSION 1.0 LANGUAGES CXX)

# ==================== C++ 标准配置 ====================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==================== 编译选项配置 ====================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

# Windows 特定编译选项（UTF-8 输入，GBK 执行）
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=utf-8 -fexec-charset=utf-8")
endif()

# 默认构建类型为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ==================== 目录配置 ====================
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/raylib-5.5_win64_mingw-w64)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ==================== 自动搜索源文件和头文件 ====================
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.h")

# ==================== 可执行文件 ====================
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 添加头文件搜索路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${INCLUDE_DIR}/core
    ${RAYLIB_DIR}/include
)

# 添加库目录
target_link_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_DIR}/lib)

# ==================== 链接库 ====================
target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
    opengl32
    gdi32
    winmm
    ws2_32
)

# Windows 特定链接选项
if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        -Wl,--stack,12582912
        -static
    )
endif()

# ==================== 编译定义 ====================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

# ==================== 复制资源文件 ====================
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
set(RESOURCE_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)

file(MAKE_DIRECTORY ${RESOURCE_OUTPUT_DIR})
file(MAKE_DIRECTORY ${RESOURCE_OUTPUT_DIR}/shaders)

# 复制所有资源文件到输出目录
file(GLOB_RECURSE RESOURCE_FILES "${RESOURCE_DIR}/*")
foreach(RES_FILE ${RESOURCE_FILES})
    file(RELATIVE_PATH REL_PATH ${RESOURCE_DIR} ${RES_FILE})
    get_filename_component(RES_DIR ${RESOURCE_OUTPUT_DIR}/${REL_PATH} DIRECTORY)
    file(MAKE_DIRECTORY ${RES_DIR})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${RES_FILE}
            ${RESOURCE_OUTPUT_DIR}/${REL_PATH}
        COMMENT "Copying resource: ${REL_PATH}"
    )
endforeach()

# ==================== 打印配置信息 ====================
message(STATUS "========================================")
message(STATUS "Project:        ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Dir:     ${SRC_DIR}")
message(STATUS "Include Dir:    ${INCLUDE_DIR}")
message(STATUS "Raylib Dir:     ${RAYLIB_DIR}")
message(STATUS "Output Dir:     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Sources:        ${SOURCES}")
message(STATUS "========================================")
