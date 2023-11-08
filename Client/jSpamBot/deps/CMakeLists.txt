project(JSON)

include(ExternalProject)

find_package (Java REQUIRED)
include (UseJava)

ExternalProject_Add(JSON-java-src
  URL               https://github.com/stleary/JSON-java/archive/refs/tags/20231013.zip
  UPDATE_COMMAND    ""
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  PATCH_COMMAND     ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_json.txt <SOURCE_DIR>/CMakeLists.txt
  INSTALL_DIR       "${CMAKE_INSTALL_PREFIX}/Client/jSpamBot"
  CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target org-json
  BUILD_BYPRODUCTS  <INSTALL_DIR>/org-json.jar
  )
