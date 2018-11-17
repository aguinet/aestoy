add_executable(aes_keyexpand aes_keyexpand.cpp)
target_link_libraries(aes_keyexpand aestoy)

add_executable(aes_process aes_process.cpp)
target_link_libraries(aes_process aestoy)

add_executable(aes_invertkeyexpand aes_invertkeyexpand.cpp)
target_link_libraries(aes_invertkeyexpand aestoy)

#add_executable(bench bench.cpp)
#target_link_libraries(bench aestoy benchmark pthread)

install(TARGETS aes_keyexpand aes_process aes_invertkeyexpand DESTINATION bin)
