cmake_minimum_required(VERSION 3.10)
project(cBankApp C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(cbank_app
    main.c menu.c clients.c comptes.c operations.c data.c utils.c globals.c)
