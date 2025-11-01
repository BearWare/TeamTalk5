project(JSON)

include(ExternalProject)

find_package (Java REQUIRED)
include (UseJava)

ExternalProject_Add(JSON-java-src
  GIT_REPOSITORY    https://github.com/stleary/JSON-java.git
  GIT_TAG           20250517
  UPDATE_COMMAND    ""
  PATCH_COMMAND     ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_json.txt <SOURCE_DIR>/CMakeLists.txt
  BUILD_COMMAND     ${CMAKE_COMMAND} --build <BINARY_DIR> --target install
  INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}
  CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  BUILD_BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/org-json.jar
  )
