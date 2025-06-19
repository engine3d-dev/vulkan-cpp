cmake_minimum_required(VERSION 3.27)
project(Starter CXX)


build_application(
    SOURCES
    application.cpp

    PACKAGES
    glfw3
    fmt

    LINK_PACKAGES
    glfw
    fmt::fmt
)
