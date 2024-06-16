
#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>

#include "os.h"

/* 2^20 pages ought to be enough for anybody */
#define NPAGES	(1024*1024)

static char* pages[NPAGES];

uint64_t alloc_page_frame(void)
{
	static uint64_t nalloc;
	uint64_t ppn;
	void* va;

	if (nalloc == NPAGES)
		errx(1, "out of physical memory");

	/* OS memory management isn't really this simple */
	ppn = nalloc;
	nalloc++;

	va = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (va == MAP_FAILED)
		err(1, "mmap failed");

	pages[ppn] = va;
	return ppn + 0xbaaaaaad;
}

void* phys_to_virt(uint64_t phys_addr)
{
	uint64_t ppn = (phys_addr >> 12) - 0xbaaaaaad;
	uint64_t off = phys_addr & 0xfff;
	char* va = NULL;

	if (ppn < NPAGES)
		va = pages[ppn] + off;

	return va;
}

int main(int argc, char **argv)
{
	uint64_t pt = alloc_page_frame();

	assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);
	page_table_update(pt, 0xcafecafeeee, 0xf00d);
	assert(page_table_query(pt, 0xcafecafeeee) == 0xf00d);
	assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);
	page_table_update(pt, 0xcafecafeeee, NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);

	    // Initial assertions to confirm no mappings exist
    assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);

    // Update table and ensure correct mappings
    page_table_update(pt, 0xcafecafeeee, 0xf00d);
    assert(page_table_query(pt, 0xcafecafeeee) == 0xf00d);
    assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);

    // Remove mapping and validate
    page_table_update(pt, 0xcafecafeeee, NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);

    // Edge Cases:
    // Large address values near the edge of 57-bit address space
    page_table_update(pt, 0x7fffffffffff, 0xdead);
    assert(page_table_query(pt, 0x7fffffffffff) == 0xdead);
    page_table_update(pt, 0x7fffffffffff, NO_MAPPING);
    assert(page_table_query(pt, 0x7fffffffffff) == NO_MAPPING);

    // Virtual addresses with different high-order bits
    page_table_update(pt, 0x10000000000, 0xbeef);
    page_table_update(pt, 0x20000000000, 0xbead);
    assert(page_table_query(pt, 0x10000000000) == 0xbeef);
    assert(page_table_query(pt, 0x20000000000) == 0xbead);
    page_table_update(pt, 0x10000000000, NO_MAPPING);
    page_table_update(pt, 0x20000000000, NO_MAPPING);
    assert(page_table_query(pt, 0x10000000000) == NO_MAPPING);
    assert(page_table_query(pt, 0x20000000000) == NO_MAPPING);

    // Boundary Testing:
    page_table_update(pt, 0x000000000000, 0x1234);
    assert(page_table_query(pt, 0x000000000000) == 0x1234);
    page_table_update(pt, 0x000000000000, NO_MAPPING);
    assert(page_table_query(pt, 0x000000000000) == NO_MAPPING);

    // Sparse mappings
    page_table_update(pt, 0x123456789012, 0xabcd);
    assert(page_table_query(pt, 0x123456789012) == 0xabcd);
    assert(page_table_query(pt, 0x123456000000) == NO_MAPPING);
    page_table_update(pt, 0x123456789012, NO_MAPPING);
    assert(page_table_query(pt, 0x123456789012) == NO_MAPPING);

    // Remapping
    page_table_update(pt, 0xabcdefabcdef, 0x1);
    assert(page_table_query(pt, 0xabcdefabcdef) == 0x1);
    page_table_update(pt, 0xabcdefabcdef, 0x2);
    assert(page_table_query(pt, 0xabcdefabcdef) == 0x2);
    page_table_update(pt, 0xabcdefabcdef, NO_MAPPING);
    assert(page_table_query(pt, 0xabcdefabcdef) == NO_MAPPING);

	    // Initial assertions for the general test cases
    assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);

    // Update and validate mappings
    page_table_update(pt, 0xcafecafeeee, 0xf00d);
    assert(page_table_query(pt, 0xcafecafeeee) == 0xf00d);
    assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);

    // Remove mapping and validate
    page_table_update(pt, 0xcafecafeeee, NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
    assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);

    // Edge Cases
    // Large address values near the edge of 57-bit space
    page_table_update(pt, 0x7fffffffffff, 0xdead);
    assert(page_table_query(pt, 0x7fffffffffff) == 0xdead);
    page_table_update(pt, 0x7fffffffffff, NO_MAPPING);
    assert(page_table_query(pt, 0x7fffffffffff) == NO_MAPPING);

    // Virtual addresses differing in high-order bits
    page_table_update(pt, 0x10000000000, 0xbeef);
    page_table_update(pt, 0x20000000000, 0xbead);
    assert(page_table_query(pt, 0x10000000000) == 0xbeef);
    assert(page_table_query(pt, 0x20000000000) == 0xbead);
    page_table_update(pt, 0x10000000000, NO_MAPPING);
    page_table_update(pt, 0x20000000000, NO_MAPPING);
    assert(page_table_query(pt, 0x10000000000) == NO_MAPPING);
    assert(page_table_query(pt, 0x20000000000) == NO_MAPPING);

    // Boundary Testing
    page_table_update(pt, 0x000000000000, 0x1234);
    assert(page_table_query(pt, 0x000000000000) == 0x1234);
    page_table_update(pt, 0x000000000000, NO_MAPPING);
    assert(page_table_query(pt, 0x000000000000) == NO_MAPPING);

    // Sparse mappings
    page_table_update(pt, 0x123456789012, 0xabcd);
    assert(page_table_query(pt, 0x123456789012) == 0xabcd);
    assert(page_table_query(pt, 0x123456000000) == NO_MAPPING);
    page_table_update(pt, 0x123456789012, NO_MAPPING);
    assert(page_table_query(pt, 0x123456789012) == NO_MAPPING);

    // Remapping
    page_table_update(pt, 0xabcdefabcdef, 0x1);
    assert(page_table_query(pt, 0xabcdefabcdef) == 0x1);
    page_table_update(pt, 0xabcdefabcdef, 0x2);
    assert(page_table_query(pt, 0xabcdefabcdef) == 0x2);
    page_table_update(pt, 0xabcdefabcdef, NO_MAPPING);
    assert(page_table_query(pt, 0xabcdefabcdef) == NO_MAPPING);

    // Assert on boundary and sparse addresses
    assert(page_table_query(pt, 0x111111111000) == NO_MAPPING);
    page_table_update(pt, 0x111111111000, 0x1aa);
    assert(page_table_query(pt, 0x111111111000) == 0x1aa);
    page_table_update(pt, 0x111111111000, NO_MAPPING);
    assert(page_table_query(pt, 0x111111111000) == NO_MAPPING);

    // Very high map and unmapping check
    page_table_update(pt, 0x7fffffffffe, 0x7aa);
    assert(page_table_query(pt, 0x7fffffffffe) == 0x7aa);
    page_table_update(pt, 0x7fffffffffe, NO_MAPPING);
    assert(page_table_query(pt, 0x7fffffffffe) == NO_MAPPING);

    // Layered mappings with different levels
    page_table_update(pt, 0x200000000005, 0x500);
    assert(page_table_query(pt, 0x200000000005) == 0x500);
    page_table_update(pt, 0x200000000006, 0x501);
    assert(page_table_query(pt, 0x200000000006) == 0x501);
    assert(page_table_query(pt, 0x200000000004) == NO_MAPPING);
    page_table_update(pt, 0x200000000005, NO_MAPPING);
    page_table_update(pt, 0x200000000006, NO_MAPPING);
    assert(page_table_query(pt, 0x200000000005) == NO_MAPPING);
    assert(page_table_query(pt, 0x200000000006) == NO_MAPPING);

    // Check if mappings maintain isolation and consistency
    page_table_update(pt, 0x30000000000a, 0x600);
    page_table_update(pt, 0x30000000000b, 0x601);
    page_table_update(pt, 0x30000000000c, 0x602);
    assert(page_table_query(pt, 0x30000000000a) == 0x600);
    assert(page_table_query(pt, 0x30000000000b) == 0x601);
    assert(page_table_query(pt, 0x30000000000c) == 0x602);
    page_table_update(pt, 0x30000000000a, NO_MAPPING);
    page_table_update(pt, 0x30000000000b, NO_MAPPING);
    page_table_update(pt, 0x30000000000c, NO_MAPPING);
    assert(page_table_query(pt, 0x30000000000a) == NO_MAPPING);
    assert(page_table_query(pt, 0x30000000000b) == NO_MAPPING);
    assert(page_table_query(pt, 0x30000000000c) == NO_MAPPING);

    // Ensure no cross-interference with higher and lower order addresses
    page_table_update(pt, 0xaaa000000001, 0x701);
    page_table_update(pt, 0xaaa000000002, 0x702);
    assert(page_table_query(pt, 0xaaa000000001) == 0x701);
    assert(page_table_query(pt, 0xaaa000000002) == 0x702);
    page_table_update(pt, 0xaaa000000001, NO_MAPPING);
    page_table_update(pt, 0xaaa000000002, NO_MAPPING);
    assert(page_table_query(pt, 0xaaa000000001) == NO_MAPPING);
    assert(page_table_query(pt, 0xaaa000000002) == NO_MAPPING);

    printf("All tests passed!\n");

	return 0;
}

