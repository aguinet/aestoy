set(TESTS
  vectors
  invert_keyschedule
)

# Compile tests
foreach(TEST ${TESTS})
  add_executable(${TEST} ${TEST}.cpp)
  target_link_libraries(${TEST} aestoy) 
  add_test(NAME ${TEST} COMMAND ${TEST})
endforeach()

