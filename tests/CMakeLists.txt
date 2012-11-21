add_executable(dynamicTest
    main.c
)
target_link_libraries(dynamicTest
    dynamic
)

add_executable(grep
    grep.c
)
target_link_libraries(grep
    dynamic
)
