// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks_posix.c -- mocked functions used in unsafe_shutdowns.c
 */

#include "unittest.h"

/*
 * pmem2_get_persist_fn - mock pmem2_get_persist_fn
 */
FUNC_MOCK(pmem2_get_persist_fn, pmem2_persist_fn, struct pmem2_map *map)
FUNC_MOCK_RUN_DEFAULT {
	return _FUNC_REAL(pmem2_get_persist_fn)(map);
}
FUNC_MOCK_END

/*
 * pmem2_deep_sync -- pmem2_deep_sync mock
 */
FUNC_MOCK(pmem2_deep_sync, int, struct pmem2_map *map, void *ptr, size_t size)
FUNC_MOCK_RUN_DEFAULT {
	return _FUNC_REAL(pmem2_deep_sync)(map, ptr, size);
}
FUNC_MOCK_END

/*
 * XXX: Unused function; pmem2_persist_mock -- replace all persist function
 
static void *
pmem2_persist_mock(const void *ptr, size_t size)
{
	return;
}
*/
