enable_testing()

find_package(GTest CONFIG)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE HEADER_FILES *.hpp *.h *.hh)

add_executable(UnitTests EXCLUDE_FROM_ALL ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(UnitTests PUBLIC GTest::GTest Grafkit::Grafkit)

set_target_properties(UnitTests PROPERTIES FOLDER tests)

add_test(
  NAME Tests
  COMMAND UnitTests
  WORKING_DIRECTORY ${TEST_DEPLOY_DIR})
