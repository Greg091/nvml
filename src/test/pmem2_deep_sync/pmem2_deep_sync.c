// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * pmem2_deep_sync.c -- unit test for pmem_deep_sync()
 *
 * usage: pmem2_deep_sync file deep_persist_size offset
 *
 */

#ifndef _WIN32
#include <sys/sysmacros.h>
#endif

#include "mmap.h"
#include "persist.h"
#include "pmem2_arch.h"
#include "pmem2_utils.h"
#include "unittest.h"

int n_msynces = 0;
int n_fences = 0;
int n_flushes = 0;
int is_devdax = 0;

#ifndef _WIN32
/*
 * pmem2_device_davx_region_find -- redefine libpmem2 function
 */
int
pmem2_device_dax_region_find(const os_stat_t *st)
{
	char dax_region_path[PATH_MAX];
	int dax_reg_id_fd;
	int reg_id;

	dev_t dev_id = st->st_rdev;

	unsigned major = major(dev_id);
	unsigned minor = minor(dev_id);
	util_snprintf(dax_region_path, PATH_MAX,
		"/sys/dev/char/%u:%u/device/dax_region/id",
		major, minor);

	if ((dax_reg_id_fd = os_open(dax_region_path, O_RDONLY)) < 0) {
		LOG(1, "!open(\"%s\", O_RDONLY)", dax_region_path);
		return PMEM2_E_ERRNO;
	}

	if (dax_reg_id_fd == 999)
		reg_id = 1;
	else
		reg_id = -1;

	return reg_id;
}
#endif

/*
 * mock_flush -- count flush calls in the test
 */
static void
mock_flush(const void *addr, size_t len)
{
	++n_flushes;
}

/*
 * mock_drain -- count drain calls in the test
 */
static void
mock_drain(void)
{
	++n_fences;
}

/*
 * pmem2_arch_init -- redefine libpmem2 function
 */
void
pmem2_arch_init(struct pmem2_arch_info *info)
{
	info->flush = mock_flush;
	info->fence = mock_drain;
}

/*
 * pmem2_get_type_from_stat -- redefine libpmem2 function
 */
int
pmem2_get_type_from_stat(const os_stat_t *st, enum pmem2_file_type *type)
{
	if (is_devdax)
		*type = PMEM2_FTYPE_DEVDAX;
	else
		*type = PMEM2_FTYPE_REG;

	return 0;
}

/*
 * pmem2_map_find -- redefine libpmem2 function
 */
struct pmem2_map *
pmem2_map_find(const void *addr, size_t len)
{
	static struct pmem2_map cur;

	cur.addr = (void *)ALIGN_DOWN((size_t)addr, Pagesize);
	if ((size_t)cur.addr < (size_t)addr)
		len += (size_t)addr - (size_t)cur.addr;

	cur.reserved_length = ALIGN_UP(len, Pagesize);

	return &cur;
}

/*
 * pmem2_flush_file_buffers_os -- redefine libpmem2 function
 */
int
pmem2_flush_file_buffers_os(struct pmem2_map *map, const void *addr, size_t len,
		int autorestart)
{
	++n_msynces;

	return 0;
}

/*
 * map_init -- fill pmem2_map in minimal scope
 */
static void
map_init(struct pmem2_map *map)
{
	const size_t length = 8 * MEGABYTE;
	map->content_length = length;
	map->addr = MALLOC(2 * length);
#ifndef _WIN32
	map->src_fd_st = MALLOC(sizeof(os_stat_t));
	/*
	 * predefined 'st_rdev' value is needed to hardcode the mocked path
	 * which is required to mock  DAX devices
	 */
	map->src_fd_st->st_rdev = 60041;
#endif
}

/*
 * map_cleanup -- cleanup pmem2_map
 */
static void
map_cleanup(struct pmem2_map *map)
{
#ifndef _WIN32
	FREE(map->src_fd_st);
#endif
	FREE(map->addr);
}

/*
 * counters_check_n_reset -- check values of counts of calls of persist
 * functions in the test and reset them
 */
static void
counters_check_n_reset(int msynces, int flushes, int fences)
{
	UT_ASSERTeq(n_msynces, msynces);
	UT_ASSERTeq(n_flushes, flushes);
	UT_ASSERTeq(n_fences, fences);

	n_msynces = 0;
	n_flushes = 0;
	n_fences = 0;
}

/*
 * test_deep_sync_func -- test pmem2_deep_sync for all granularity options
 */
static int
test_deep_sync_func(const struct test_case *tc, int argc, char *argv[])
{
	struct pmem2_map map;
	map_init(&map);

	void *addr = map.addr;
	size_t len = map.content_length;

	map.effective_granularity = PMEM2_GRANULARITY_PAGE;
	pmem2_set_flush_fns(&map);
	map.persist_fn(addr, len);
	int ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 0, 0);

	map.effective_granularity = PMEM2_GRANULARITY_CACHE_LINE;
	pmem2_set_flush_fns(&map);
	map.persist_fn(addr, len);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 1, 1);

	map.effective_granularity = PMEM2_GRANULARITY_BYTE;
	pmem2_set_flush_fns(&map);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 1, 1);

	map_cleanup(&map);

	return 0;
}

/*
 * test_deep_sync_func_devdax -- test pmem2_deep_sync with mocked DAX devices
 */
static int
test_deep_sync_func_devdax(const struct test_case *tc, int argc, char *argv[])
{
	struct pmem2_map map;
	map_init(&map);

	void *addr = map.addr;
	size_t len = map.content_length;

	is_devdax = 1;
	map.effective_granularity = PMEM2_GRANULARITY_CACHE_LINE;
	pmem2_set_flush_fns(&map);
	map.persist_fn(addr, len);
	int ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(0, 1, 1);

	map.effective_granularity = PMEM2_GRANULARITY_BYTE;
	pmem2_set_flush_fns(&map);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(0, 1, 1);

	map_cleanup(&map);

	return 0;
}

/*
 * test_deep_sync_addr_beyond_mapping -- test pmem2_deep_sync with the address
 * that goes beyond mapping
 */
static int
test_deep_sync_addr_beyond_mapping(const struct test_case *tc, int argc,
					char *argv[])
{
	struct pmem2_map map;

	map_init(&map);

	void *addr = (void *)((uintptr_t)map.addr + map.content_length + 1);
	size_t len = map.content_length;

	int ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, PMEM2_E_SYNC_RANGE);

	map_cleanup(&map);

	return 0;
}

/*
 * test_deep_sync_range_beyond_mapping -- test pmem2_deep_sync with a range
 * that is partially outside the mapping
 */
static int
test_deep_sync_range_beyond_mapping(const struct test_case *tc, int argc,
					char *argv[])
{
	struct pmem2_map map;
	map_init(&map);

	void *addr = (void *)((uintptr_t)map.addr + map.content_length / 2);
	size_t len = map.content_length;

	map.effective_granularity = PMEM2_GRANULARITY_PAGE;
	pmem2_set_flush_fns(&map);
	map.persist_fn(addr, len);
	int ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 0, 0);

	map.effective_granularity = PMEM2_GRANULARITY_CACHE_LINE;
	pmem2_set_flush_fns(&map);
	map.persist_fn(addr, len);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 1, 1);

	map.effective_granularity = PMEM2_GRANULARITY_BYTE;
	pmem2_set_flush_fns(&map);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 1, 1);

	/* check cases when given address is equal end of mapping */
	addr = (void *)((uintptr_t)map.addr + map.content_length);

	map.effective_granularity = PMEM2_GRANULARITY_PAGE;
	pmem2_set_flush_fns(&map);
	map.persist_fn(addr, len);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 0, 0);

	map.effective_granularity = PMEM2_GRANULARITY_CACHE_LINE;
	pmem2_set_flush_fns(&map);
	map.persist_fn(addr, len);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 1, 1);

	map.effective_granularity = PMEM2_GRANULARITY_BYTE;
	pmem2_set_flush_fns(&map);
	ret = pmem2_deep_sync(&map, addr, len);
	UT_ASSERTeq(ret, 0);
	counters_check_n_reset(1, 1, 1);

	map_cleanup(&map);

	return 0;
}

/*
 * test_cases -- available test cases
 */
static struct test_case test_cases[] = {
	TEST_CASE(test_deep_sync_func),
	TEST_CASE(test_deep_sync_func_devdax),
	TEST_CASE(test_deep_sync_addr_beyond_mapping),
	TEST_CASE(test_deep_sync_range_beyond_mapping),
};

#define NTESTS (sizeof(test_cases) / sizeof(test_cases[0]))

int
main(int argc, char *argv[])
{
	START(argc, argv, "pmem2_deep_sync");
	pmem2_persist_init();
	util_init();
	TEST_CASE_PROCESS(argc, argv, test_cases, NTESTS);
	DONE(NULL);
}
