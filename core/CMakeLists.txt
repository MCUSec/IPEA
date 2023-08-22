aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}" IPEA_SRCS)

add_library(
	IPEA
	${IPEA_SRCS}
)

target_compile_definitions(IPEA PRIVATE
	-DTARGET_ENABLE_FUZZ
	-DENABLE_SANITIZER
)


target_compile_options(IPEA PRIVATE -O3)
# target_compile_options(IPEA PRIVATE -Og -g -fsanitize=address)

target_include_directories(
	IPEA
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../include/host"
)
