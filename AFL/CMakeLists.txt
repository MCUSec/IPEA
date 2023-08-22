set(AFL_EXEC "ipea-fuzz")
set(AFL_GCC "ipea-gcc")
set(AFL_AS "ipea-as")

set(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/afl-fuzz.c")
add_executable(${AFL_EXEC} ${SRCS})

add_executable(${AFL_GCC} 
	"${CMAKE_CURRENT_SOURCE_DIR}/afl-gcc.c"
)

add_executable(${AFL_AS} 
	"${CMAKE_CURRENT_SOURCE_DIR}/afl-as.c"
)

target_compile_options(${AFL_EXEC} PRIVATE -O3)

target_include_directories(${AFL_EXEC}
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../include/host"
)

target_compile_definitions(${AFL_GCC} PRIVATE AFL_PATH="/usr/bin" BIN_PATH="/usr/bin")

target_compile_definitions(${AFL_AS} PRIVATE BIN_PATH="/usr/bin")

# target_link_options(${AFL_EXEC} PRIVATE -fsanitize=address)

target_link_libraries(${AFL_EXEC} PRIVATE 
	IPEA
	jlinkarm
	pthread
	stdc++
	rt
	m
	dl
	jsoncpp
	elf
	config
)
