// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks_posix.c -- mocked functions used in pmem2_deep_sync_dax.c
 * (Posix implementation)
 */

#include "unittest.h"

#define BUS_DEVICE_PATH "/sys/bus/nd/devices/region1/deep_flush"
#define DEV_DEVICE_PATH "/sys/dev/char/234:137/device/dax_region/id"

/*
 * open -- open mock
 */
FUNC_MOCK(os_open, int, const char *path, int flags, ...)
FUNC_MOCK_RUN_DEFAULT {
	if (strcmp(path, DEV_DEVICE_PATH) == 0 ||
			strcmp(path, BUS_DEVICE_PATH) == 0) {
		UT_OUT("mocked open, path %s", path);
		if (os_access(path, R_OK))
			return 999;
	}

	va_list ap;
	va_start(ap, flags);
	int mode = va_arg(ap, int);
	va_end(ap);

	return _FUNC_REAL(os_open)(path, flags, mode);
}
FUNC_MOCK_END

/*
 * write -- write mock
 */
FUNC_MOCK(write, int, int fd, const void *buffer, size_t count)
FUNC_MOCK_RUN_DEFAULT {
	if (fd == 999) {
		UT_OUT("mocked write, path %d", fd);
		return 1;
	}
	return _FUNC_REAL(write)(fd, buffer, count);
}
FUNC_MOCK_END
