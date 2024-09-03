cmake_minimum_required(VERSION 3.5)

project(ArchiverOS LANGUAGES C)

add_executable(ArchiverOS main.c)

include(GNUInstallDirs)
install(TARGETS ArchiverOS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
