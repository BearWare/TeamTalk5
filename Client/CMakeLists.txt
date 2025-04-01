project (TeamTalkClients)

if (MSVC)

  include(ExternalProject)

  option (BUILD_TEAMTALK_CLIENT_TOLK "Build TeamTalk clients with Tolk" ON)
  
  ExternalProject_Add(tolk-src
    GIT_REPOSITORY    https://github.com/bear101/tolk
    GIT_TAG           1e538e2
    UPDATE_COMMAND    ""
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DTOLK_WITH_JAVAJNI=OFF -DTOLK_MULTITHREADED=ON
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Release
    INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/Tolk
    INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Release
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/Tolk.lib
    )
  ExternalProject_Get_Property(tolk-src INSTALL_DIR)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  
  add_library(Tolk STATIC IMPORTED GLOBAL)
  add_dependencies(Tolk tolk-src)
  target_include_directories (Tolk INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(Tolk PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/Tolk.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/Tolk.lib)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set (TOLK_DLL_FILES ${INSTALL_DIR}/lib/nvdaControllerClient64.dll ${INSTALL_DIR}/lib/SAAPI64.dll ${INSTALL_DIR}/lib/Tolk.dll)
  else()
    set (TOLK_DLL_FILES ${INSTALL_DIR}/lib/dolapi32.dll ${INSTALL_DIR}/lib/nvdaControllerClient32.dll ${INSTALL_DIR}/lib/SAAPI32.dll ${INSTALL_DIR}/lib/Tolk.dll)
  endif()
  
  add_subdirectory (TeamTalkClassic)
  if (CMAKE_CSharp_COMPILER)
    add_subdirectory (TeamTalkApp.NET)
    add_subdirectory (ttserverlog.NET)
  else()
    message(WARNING "No C# compiler detected. Skipping .NET client examples")
  endif()
endif()

add_subdirectory (jSpamBot)
add_subdirectory (jTeamTalk)
add_subdirectory (qtTeamTalk)
add_subdirectory (ttserverlog)

