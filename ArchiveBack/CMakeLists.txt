cmake_minimum_required(VERSION 3.5)

project(ArchiveBack LANGUAGES C)

add_executable(ArchiveBack main.c)

include(GNUInstallDirs)
install(TARGETS ArchiveBack
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
