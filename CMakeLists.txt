cmake_minimum_required(VERSION 3.20.0)

# Pull in nRF Connect SDK / Zephyr build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(presence_relay
    VERSION 1.0.0
    DESCRIPTION "Zero-Knowledge Presence Relay - Civic OS Project"
    LANGUAGES C
)

# Application sources
target_sources(app PRIVATE
    src/main.c
)

# Include directories
target_include_directories(app PRIVATE
    src/
)
