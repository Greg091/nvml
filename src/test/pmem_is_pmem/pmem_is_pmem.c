/*
 * Copyright 2019, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * pmem_is_pmem.c -- unit test for pmem_is_pmem()
 *
 * usage: pmem_is_pmem <test_case> <path> <size>
 */

#include "unittest.h"

#define PAGE_4K ((uintptr_t)1 << 12)

static void
test_discontinuous_mapping(const char *path, size_t size)
{
	void *pmem_addr;
	size_t mapped_len;
	int is_pmem;

	pmem_addr = pmem_map_file(path, size, PMEM_FILE_CREATE, 0666,
				&mapped_len, &is_pmem);
	UT_ASSERTne(pmem_addr, NULL);
	UT_ASSERTeq(pmem_is_pmem(pmem_addr, size), 1);

	void *pmem_addr_range = (char *)pmem_addr + size;
	void *mem_addr = pmem_addr;

	for (; mem_addr < pmem_addr_range;
	    mem_addr = (char *)mem_addr + PAGE_4K) {
		UT_ASSERTeq(pmem_is_pmem(mem_addr, 1), 1);
		pmem_unmap(mem_addr, 1);
	}
	UT_ASSERTne(pmem_is_pmem(pmem_addr, size), 1);
}

static void
test_mmap(const char *path, size_t size)
{
	int f = open(path, O_CREAT, 0640);
	void *addr = (void *)strtoull("0x7fff77424000", NULL, 0);

	mmap(addr, size, PROT_EXEC, MAP_SHARED, f, 0);
	UT_ASSERTeq(pmem_is_pmem(addr, 1), 0);

	close(f);
}

int
main(int argc, char *argv[])
{
	START(argc, argv, "pmem_is_pmem");

	if (argc != 4)
		UT_FATAL("usage: %s <test_case> <path> <size>", argv[0]);

	int test_case = atoi(argv[1]);
	const char *path = argv[2];
	size_t mem_size = strtoull(argv[3], NULL, 0);

	switch (test_case) {
		case 1:
			test_discontinuous_mapping(path, mem_size);
			break;

		case 2:
			test_mmap(path, mem_size);
			break;
	}

	DONE(NULL);
}
