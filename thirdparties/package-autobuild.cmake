find_package(
  Python3
  COMPONENTS Interpreter
  REQUIRED)

execute_process(COMMAND ${Python3_EXECUTABLE} thirdparties/vendor.py init
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

execute_process(COMMAND ${Python3_EXECUTABLE} thirdparties/vendor.py build all
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
