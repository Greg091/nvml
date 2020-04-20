#!../env.py
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

import testframework as t


class PMEM2_MAP_PROT(t.Test):
    test_type = t.Short
    filesize = 16 * t.MiB

    def run(self, ctx):
        filepath = ctx.create_holey_file(self.filesize, 'testfile',)
        ctx.exec('pmem2_map_prot', self.test_case, filepath)


class TEST0(PMEM2_MAP_PROT):
    """
    READ/WRITE protection on file opened in read-write mode - should succeed
    """
    test_case = "test_rw_mode_rw_prot"


class TEST1(PMEM2_MAP_PROT):
    """
    READ/WRITE protection on file opened in read-only mode - should fail
    """
    test_case = "test_r_mode_rw_prot"


@t.windows_only
class TEST2(PMEM2_MAP_PROT):
    """
    READ/WRITE protection on file opened in read-only mode - should fail
    """
    test_case = "test_rw_modex_rwx_prot"


@t.windows_only
class TEST3(PMEM2_MAP_PROT):
    """
    READ/WRITE protection on file opened in read-only mode - should fail
    """
    test_case = "test_rw_modex_rx_prot"


class TEST4(PMEM2_MAP_PROT):
    """
    READ protection on file opened in read-write mode - should succeed
    """
    test_case = "test_rw_mode_r_prot"


class TEST5(PMEM2_MAP_PROT):
    """
    READ protection on file opened in read-only mode - should succeed
    """
    test_case = "test_r_mode_r_prot"


# PAGE_NOACCESS flag is not supported by the CreateFileMapping function.
# The test will fail under memcheck as expected
@t.windows_exclude
@t.require_valgrind_disabled('memcheck')
class TEST6(PMEM2_MAP_PROT):
    """
    NONE_PROTECTION on file opened in read-write mode - should succeed
    """
    test_case = "test_rw_mode_none_prot"


class TEST7(PMEM2_MAP_PROT):
    """
    READ|EXEC protection on file opened in read|write|exec mode; test runs
    the program, which is put in mapped memory - should succeed
    """
    test_case = "test_rx_mode_rx_prot_do_execute"


class PMEM2_PROT_EXEC(t.Test):
    test_type = t.Short
    filesize = 16 * t.MiB
    is_map_private = 0

    def run(self, ctx):
        filepath = ctx.create_holey_file(self.filesize, 'testfile')
        ctx.exec('pmem2_map_prot', self.test_case,
                 filepath, self.is_map_private)


class TEST8(PMEM2_PROT_EXEC):
    """
    READ|EXEC protection on file opened in read|write|exec mode; test writes
    data to mapped memory - should failed
    """
    test_case = "test_rwx_mode_rx_prot_do_write"


class TEST9(PMEM2_PROT_EXEC):
    """
    READ|EXEC protection on file opened in read|write|exec mode; test writes
    data to mapped memory with MAP_PRIVATE - should failed
    """
    test_case = "test_rwx_mode_rx_prot_do_write"
    is_map_private = 1


class TEST10(PMEM2_PROT_EXEC):
    """
    READ|WRITE|EXEC protection on file opened in read|write|exec mode; test
    runs the program, which is put in mapped memory - should succeed
    """
    test_case = "test_rwx_mode_rwx_prot_do_execute"


class TEST11(PMEM2_PROT_EXEC):
    """
    READ|WRITE protection on file opened in read|write mode; test runs
    the program, which is put in mapped memory with MAP_PRIVATE -
    should succeed
    """
    test_case = "test_rwx_mode_rwx_prot_do_execute"
    is_map_private = 1


class TEST12(PMEM2_PROT_EXEC):
    """
    READ|EXEC protection on file opened in read|write mode; test runs
    the program, which is put in mapped memory - should failed
    """
    test_case = "test_rw_mode_rw_prot_do_execute"


class TEST13(PMEM2_PROT_EXEC):
    """
    READ|EXEC protection on file opened in read|write mode; test runs
    the program, which is put in mapped memory with MAP_PRIVATE - should failed
    """
    test_case = "test_rw_mode_rw_prot_do_execute"
    is_map_private = 1


class TEST14(PMEM2_MAP_PROT):
    """
    READ|EXEC protection on file opened in read|exec|write mode; test
    runs the program, which is put in mapped memory with MAP_PRIVATE -
    should succeed
    """
    test_case = "test_rwx_prot_map_priv_do_execute"


@t.windows_exclude
@t.require_devdax(t.DevDax('devdax1'))
class PMEM2_MAP_DEVDAX(t.Test):
    test_type = t.Short

    def run(self, ctx):
        dd = ctx.devdaxes.devdax1
        ctx.exec('pmem2_map_prot', self.test_case, dd.path)


class TEST15(PMEM2_MAP_DEVDAX):
    """
    READ/WRITE protection on device DAX opened in read-write
    mode - should succeed
    """
    test_case = "test_rw_mode_rw_prot"


class TEST16(PMEM2_MAP_DEVDAX):
    """
    READ|EXEC protection on device DAX opened in read|write|exec mode; test
    runs the program, which is put in mapped memory - should succeed
    """
    test_case = "test_rx_mode_rx_prot_do_execute"
