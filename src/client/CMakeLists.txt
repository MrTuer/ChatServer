##定义一个SRC_LIST变量包含该目录下所有的源文件
aux_source_directory(. SRC_LIST)

add_executable(ChatClient ${SRC_LIST})


target_link_libraries(ChatClient pthread)
