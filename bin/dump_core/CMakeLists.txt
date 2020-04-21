add_executable(dump_core
  dump_core.cpp
)

target_link_libraries(dump_core
  PUBLIC
    rvncorevirtualbox
)

include(GNUInstallDirs)
install(TARGETS dump_core
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
