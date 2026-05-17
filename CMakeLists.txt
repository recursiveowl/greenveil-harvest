cmake_minimum_required(VERSION 3.20)
project(GreenveilHarvest VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS System Window Graphics Audio)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} "src/pch.h"   "src/ui/MarketUI.h" "src/systems/FishingSystem.h" "src/ui/FishingUI.h" "src/systems/BuildingSystem.h" "src/systems/BuildingSystem.cpp" "src/ui/BuildingUI.h" "src/ui/BuildingUI.cpp" "src/world/CoinPickup.h" "src/world/CoinPickup.cpp" "src/core/UIScale.h" "src/world/InteractTile.h" "src/world/InteractTile.cpp" "src/ui/InteractPrompt.h" "src/ui/InteractPrompt.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/include"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src/pch.h"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::System
    SFML::Window
    SFML::Graphics
    SFML::Audio
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)