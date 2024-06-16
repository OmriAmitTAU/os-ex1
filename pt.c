#include <stdio.h>
#include <inttypes.h>

#include "os.h"

#define VPN_SIZE 45
#define INDEX_SIZE 9
#define LEVELS (VPN_SIZE / INDEX_SIZE)

int is_valid(uint64_t *page_table, uint64_t index) {
    return (page_table[index] & 1) == 1;
}

uint64_t extract_index_by_level(uint64_t vpn, int level) {
    return (vpn >> (level * INDEX_SIZE)) & 0x1ff;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    uint64_t *page_table, index;
    page_table = phys_to_virt(pt << 12);

    for (int level = LEVELS-1; level > 0; level--) {
        index = extract_index_by_level(vpn, level);
        if (!is_valid(page_table, index)) {
            if (ppn == NO_MAPPING) {
                return;
            }
            uint64_t new_page = alloc_page_frame();
            page_table[index] = (new_page << 12) | 1;
        }
        page_table = phys_to_virt(page_table[index] -1);
    }
    index = extract_index_by_level(vpn, 0);
    if (ppn == NO_MAPPING) {
        page_table[index] = 0;
        return;
    }
    page_table[index] = (ppn << 12) | 1;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t *page_table, index;
    page_table = phys_to_virt(pt << 12);

    for (int level = LEVELS-1; level > 0; level--) {
        index = extract_index_by_level(vpn, level);
        if (!is_valid(page_table, index)) {
            return NO_MAPPING;
        }
        page_table = phys_to_virt(page_table[index] -1);
    }
    index = extract_index_by_level(vpn, 0);
    if (is_valid(page_table, index)) {
        return page_table[index] >> 12;
    }
    return NO_MAPPING;
}
