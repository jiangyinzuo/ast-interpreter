cmake_minimum_required(VERSION 3.1.0)
project(assign1)

find_package(Clang REQUIRED CONFIG HINTS ${LLVM_DIR} ${LLVM_DIR}/lib/cmake/clang NO_DEFAULT_PATH)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE "./*.cpp")
add_library(ast-interpreter-lib ${SOURCE})
target_link_libraries(ast-interpreter-lib ${LLVM_LIBRARY_DIRS})
target_include_directories(ast-interpreter-lib PUBLIC ${LLVM_INCLUDE_DIRS})
# include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS} SYSTEM)
# link_directories(${LLVM_LIBRARY_DIRS})

target_include_directories(ast-interpreter-lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_executable(ast-interpreter cmd/ASTInterpreter.cpp)
target_include_directories(ast-interpreter PUBLIC ${PROJECT_SOURCE_DIR}/include)

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Option
  Support
  )


target_link_libraries(ast-interpreter-lib
  clangAST
  clangBasic
  clangFrontend
  clangTooling
  )
target_link_libraries(ast-interpreter ast-interpreter-lib)

install(TARGETS ast-interpreter
  RUNTIME DESTINATION bin)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_subdirectory(unittest)
endif()
