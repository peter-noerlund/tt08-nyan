cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})

project(tt08-nyan LANGUAGES CXX)

find_package(Git REQUIRED)
find_package(Python COMPONENTS Interpreter REQUIRED)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/user_config.json
    COMMAND . venv/bin/activate && tt/tt_tool.py --create-user-config
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/config.json ${CMAKE_CURRENT_SOURCE_DIR}/info.yaml ${CMAKE_CURRENT_SOURCE_DIR}/tt/tt_tool.py ${CMAKE_CURRENT_SOURCE_DIR}/venv/bin/python
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tt/tt_tool.py
    COMMAND ${GIT_EXECUTABLE} clone https://github.com/TinyTapeout/tt-support-tools tt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/venv/bin/python
    COMMAND ${Python_EXECUTABLE} -m venv venv
    COMMAND venv/bin/pip install --upgrade pip
    COMMAND venv/bin/pip install -r tt/requirements.txt
    COMMAND venv/bin/pip install librelane==3.0.3
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tt/tt_tool.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    harden
    COMMAND . venv/bin/activate && tt/tt_tool.py --harden
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/user_config.json ${CMAKE_CURRENT_SOURCE_DIR}/tt/tt_tool.py ${CMAKE_CURRENT_SOURCE_DIR}/venv/bin/python
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    create-png
    COMMAND . venv/bin/activate && tt/tt_tool.py --create-png
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tt/tt_tool.py ${CMAKE_CURRENT_SOURCE_DIR}/venv/bin/python
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    create-pdf
    COMMAND . venv/bin/activate && tt/tt_tool.py --create-pdf
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tt/tt_tool.py ${CMAKE_CURRENT_SOURCE_DIR}/venv/bin/python
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    create-summary
    COMMAND . venv/bin/activate && tt/tt_tool.py --print-warnings > ${CMAKE_CURRENT_BINARY_DIR}/summary.md
    COMMAND . venv/bin/activate && tt/tt_tool.py --print-stats >> ${CMAKE_CURRENT_BINARY_DIR}/summary.md
    COMMAND scripts/clock_summary.py >> ${CMAKE_CURRENT_BINARY_DIR}/summary.md
    COMMAND . venv/bin/activate && tt/tt_tool.py --print-cell-summary >> ${CMAKE_CURRENT_BINARY_DIR}/summary.md
    COMMAND . venv/bin/activate && tt/tt_tool.py --print-cell-category >> ${CMAKE_CURRENT_BINARY_DIR}/summary.md
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tt/tt_tool.py ${CMAKE_CURRENT_SOURCE_DIR}/venv/bin/python
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tools)
add_subdirectory(sim)
add_subdirectory(pico-ice)
