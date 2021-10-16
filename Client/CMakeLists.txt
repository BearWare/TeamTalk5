cmake_minimum_required(VERSION 3.1)

project (TeamTalkClients)

if (MSVC)

  include(ExternalProject)

  option (BUILD_TEAMTALK_CLIENT_TOLK "Build TeamTalk clients with Tolk" ON)
  
  ExternalProject_Add(tolk-src
    GIT_REPOSITORY    https://github.com/bear101/tolk
    GIT_TAG           6544167
    UPDATE_COMMAND    ""
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DTOLK_WITH_JAVAJNI=OFF
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Release
    INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/Tolk
    INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Release
    )
  ExternalProject_Get_Property(tolk-src INSTALL_DIR)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  
  add_library(Tolk STATIC IMPORTED GLOBAL)
  add_dependencies(Tolk tolk-src)
  target_include_directories (Tolk INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(Tolk PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/Tolk.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/Tolk.lib)

  add_subdirectory (TeamTalkClassic)
  add_subdirectory (TeamTalkApp.NET)
  add_subdirectory (ttserverlog.NET)
endif()

add_subdirectory (qtTeamTalk)
add_subdirectory (ttserverlog)

