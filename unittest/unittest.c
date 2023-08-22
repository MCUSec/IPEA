#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bridge.h"
#include "init.h"
#include "device_config.h"
#include "common.h"
#include "JLinkARMDLL.h"

static const char *input_path;

static const char *target_path;

static const char *probe_conf_path = "jlink.conf";

static bool skip_download;

static bool profiling;

static int target_timeout = 1000;

static uint32_t download_addr = 0;

void usage(const char *argv0)
{
	fprintf(stderr, "usage: %s <target> [-i <input_path>] "
					"[-c <probe_config_path>] "
					"[-l <download_address>] " 
					"[-t <timeout>] [-n]\n", argv0);
	exit(1);
}


bool load_testcase(const target_info_t *target_info, const char *path, unsigned char *testcase_buf, U32 *testcase_len)
{
	int fd, nread;

	if (path && strlen(path) > 0) {
		fd = open(path, O_RDONLY);
	} else {
		fd = STDIN_FILENO;
	}

	if (fd == -1)
		return false;

	nread = read(fd, testcase_buf, target_info->p_fuzz_inpbuf_size);
	*testcase_len = (U32)nread;
	
	return nread > 0;
}


void print_usage(char *argv0)
{
	usage(argv0);
	exit(-1);
}

void parse_cmd(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "Npc:i:t:l:")) != -1) {
		switch (opt) {
		case 'N':
			skip_download = true;
			break;
		case 'p':
			profiling = true;
			break;
		case 'c':
			probe_conf_path = optarg;
			break;
		case 'i':
			input_path = optarg;
			break;
		case 't':
			target_timeout = atoi(optarg);
			break;
		case 'l':
			download_addr = strtoul(optarg, NULL, 16);
			break;
		case '?':
			print_usage(argv[0]);
		}
	}

	if (argc - optind < 1) {
		print_usage(argv[0]);
	}

	target_path = argv[optind];
}

int main(int argc, char *argv[])
{
	target_info_t target_info;
	INIT_PARAS paras;
	unsigned char *testcase = NULL;
	U32 tc_size = 0;
	unsigned int runtime;
	int r = 0;

	memset(&paras, 0, sizeof(INIT_PARAS));
    parse_cmd(argc, argv);
	
	if (!parse_target(target_path, &target_info)) {
		fprintf(stderr, "failed to parse target binary\n");
		return -1;
	}

	if (TARGET_IN_FUZZ_MODE(&target_info)) {
		testcase = malloc(target_info.p_fuzz_inpbuf_size);
		if (!testcase) {
			fprintf(stderr, "failed to allocate memory for input\n");
			return -1;
		}

		memset(testcase, 0, target_info.p_fuzz_inpbuf_size);

		if (!load_testcase(&target_info, input_path, testcase, &tc_size)) {
			fprintf(stderr, "failed to get input\n");
			free(testcase);
			return -1;
		}
	}

	if (profiling && !target_info.p_max_stack_usage) {
		printf("not profiling\n");
		profiling = false;
	}

	AFL_JLinkInit(&target_info, probe_conf_path, skip_download);

	// FIXME
	Context_GlobalInit();
	
	r = AFL_RunTarget(&target_info, 0, testcase, tc_size, target_timeout, &runtime, NULL, false, profiling, true);

	printf("Total runtime: %u ms\n", runtime);

	switch (r) {
	case TARGET_NORMAL:
		printf("Target terminated normally\n");
		break;	
	case TARGET_CRASH:
		RTT_DumpCallstack("callstack.txt");
		printf("Target crashed\n");
		break;
	case TARGET_TIMEOUT:
		printf("Target is timeout\n");
		break;
	default:
		fprintf(stderr, "Error ocurred. Please try again.\n");
		break;
	}
	
	if (testcase) {
		free(testcase);
		testcase = NULL;
	}
	
	AFL_JLinkShutdown(&target_info);
    return r;
}
