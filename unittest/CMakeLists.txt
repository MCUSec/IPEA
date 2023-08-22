set(UNITTEST_EXEC "ipea-unittest")

set(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/unittest.c")

add_executable(${UNITTEST_EXEC} ${SRCS})

target_compile_options(${UNITTEST_EXEC} PRIVATE -O3)
# target_compile_options(${UNITTEST_EXEC} PRIVATE -Og -g -fsanitize=address)

# target_link_options(${UNITTEST_EXEC} PRIVATE -fsanitize=address)

target_include_directories(${UNITTEST_EXEC}
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../include/host"
)

target_link_libraries(${UNITTEST_EXEC} PRIVATE 
	IPEA
	jlinkarm
	pthread
	stdc++
	rt
	m
	dl
	elf
	config
	jsoncpp
)
