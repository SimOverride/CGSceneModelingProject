cmake_minimum_required(VERSION 3.20)

project(scene_modeling LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_PATH ${CMAKE_SOURCE_DIR}/src)
set(THIRD_PARTY_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/external)
set(SHADER_TARGET_PATH ${CMAKE_BINARY_DIR}/media/shader)

file(GLOB PROJECT_SHADERS src/shader/*.vert src/shader/*.geom src/shader/*.frag)
file(COPY "media/" DESTINATION "media")

add_subdirectory(${THIRD_PARTY_LIBRARY_PATH}/glm)
add_subdirectory(${THIRD_PARTY_LIBRARY_PATH}/glad)
add_subdirectory(${THIRD_PARTY_LIBRARY_PATH}/glfw)
add_subdirectory(${THIRD_PARTY_LIBRARY_PATH}/imgui)
add_subdirectory(${THIRD_PARTY_LIBRARY_PATH}/stb)

file(GLOB BASE_HDR ${SOURCE_PATH}/base/*.h)
file(GLOB BASE_SRC ${SOURCE_PATH}/base/*.cpp)
file(GLOB PROJECTS_HDR ${SOURCE_PATH}/*.h)
file(GLOB PROJECTS_SRC ${SOURCE_PATH}/*.cpp)

add_executable(scene_modeling ${BASE_HDR} ${BASE_SRC} ${PROJECTS_HDR} ${PROJECTS_SRC})

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
elseif(UNIX)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Debug")
    else()
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Release")
    endif()
endif()

target_include_directories(
    scene_modeling PUBLIC
    ${THIRD_PARTY_LIBRARY_PATH}/glm
    ${THIRD_PARTY_LIBRARY_PATH}/glfw/include
    ${THIRD_PARTY_LIBRARY_PATH}/glad/include
    ${THIRD_PARTY_LIBRARY_PATH}/imgui
    ${THIRD_PARTY_LIBRARY_PATH}/stb
)

include("cmake/hardlink_shaders.cmake")
hardlink_shaders(${PROJECT_NAME} ${SHADER_TARGET_PATH} PROJECT_SHADERS)

target_link_libraries(scene_modeling PUBLIC glm glfw glad imgui stb)