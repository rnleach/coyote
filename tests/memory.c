#include "test.h"

/*--------------------------------------------------------------------------------------------------------------------------
 *
 *                                                    Tests for Memory
 *
 *-------------------------------------------------------------------------------------------------------------------------*/
static void
test_allocate_free(void)
{
    CoyMemoryBlock mem = coy_memory_allocate(1);
    Assert(mem.valid);
    coy_memory_free(&mem);
    Assert(!mem.valid);

    mem = coy_memory_allocate(COY_KiB(1));
    Assert(mem.valid);

    u8 *byte = mem.mem;
    for(size i = 0; i < mem.size; ++i) { byte[i] = (i & 0xFF); }
    for(size i = 0; i < mem.size; ++i) { Assert(byte[i] == (i & 0xFF)); }

    coy_memory_free(&mem);
    Assert(!mem.valid);

    mem = coy_memory_allocate(COY_MiB(1));
    Assert(mem.valid);

    byte = mem.mem;
    for(size i = 0; i < mem.size; ++i) { byte[i] = (i & 0xFF); }
    for(size i = 0; i < mem.size; ++i) { Assert(byte[i] == (i & 0xFF)); }

    coy_memory_free(&mem);
    Assert(!mem.valid);

    mem = coy_memory_allocate(COY_GiB(1));
    Assert(mem.valid);

    byte = mem.mem;
    for(size i = 0; i < mem.size; ++i) { byte[i] = (i & 0xFF); }
    for(size i = 0; i < mem.size; ++i) { Assert(byte[i] == (i & 0xFF)); }

    coy_memory_free(&mem);
    Assert(!mem.valid);

}

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                     All file Memory
 *-------------------------------------------------------------------------------------------------------------------------*/
void
coyote_memory_tests(void)
{
    test_allocate_free();
}

