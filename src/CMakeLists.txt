add_library(aestoy
  aestoy.cpp
  tools.cpp
)

install(TARGETS aestoy
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES 
  ../include/aestoy/aestoy.h
  DESTINATION include/aestoy)
