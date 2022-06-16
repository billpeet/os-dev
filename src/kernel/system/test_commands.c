#include "test_commands.h"

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../drivers/vga.h"
#include "../drivers/ata.h"
#include "../paging.h"
#include "../frame_allocator.h"
#include "../x86.h"

int try_run_test_cmd(const char *curr_cmd)
{
    if (!strcasecmp(curr_cmd, "newframe"))
    {
        void *new_frame = allocate_frame();
        printf("New frame: 0x%p\n", new_frame);
        // for (uint64_t i = 0;; i++) // allocate all frames
        // {
        //     if (allocate_frame() == 0 && i != 0)
        //     {
        //         printf("Allocated ");
        //         writeInt(i);
        //         printf(" frames\n");
        //         break;
        //     }
        // }
    }
    else if (!strcasecmp(curr_cmd, "deaframe"))
    {
        void *new_frame1 = allocate_frame();
        void *new_frame2 = allocate_frame();
        void *new_frame3 = allocate_frame();
        printf("New frame: 0x%p\n", new_frame3);
        printf("Now deallocating again\n");
        deallocate_frame(new_frame3);
        deallocate_frame(new_frame2);
        deallocate_frame(new_frame1);
        void *new_new_frame = allocate_frame();
        printf("Next frame: 0x%p\n", new_new_frame);
    }
    else if (!strcasecmp(curr_cmd, "paging"))
    {
        size_t ptr = 0x00000fffffeee010;
        ptr = 0x20003;
        size_t phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: 0x%p\n", phys_addr);
    }
    else if (!strcasecmp(curr_cmd, "recursive"))
    {
        size_t l4_table_addr = (size_t)level_4_table();
        page_table_t *p4 = (page_table_t *)0xfffffffffffff000;
        size_t phys_addr = get_physaddr(p4);
        printf("P4 table: 0x%p, phys_addr:0x%x, actual: 0x%u\n", p4, phys_addr, l4_table_addr);
        printf("Attempting to deallocate: %x\n", p4->entries[0]);
    }
    else if (!strcasecmp(curr_cmd, "newpage"))
    {
        size_t ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        printf("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        printf("Getting phys addr\n");
        size_t phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: %p\n", phys_addr);
    }
    else if (!strcasecmp(curr_cmd, "newhuge"))
    {
        size_t ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        printf("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10000010); // huge
        printf("Getting phys addr\n");
        size_t phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: %p\n", phys_addr);
    }
    else if (!strcasecmp(curr_cmd, "unmap"))
    {
        size_t ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        printf("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        printf("Getting phys addr\n");
        size_t phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: %p\n", phys_addr);
        int *int_ptr = (int *)ptr;
        printf("Here's its value: %i\n", int_ptr);
        unmap_page((void *)ptr);
        // printf("Unmapped, now attempting to get physical addr:\n");
        // get_physaddr((void *)ptr);
        // printf("%i\n", *int_ptr);
    }
    else if (!strcasecmp(curr_cmd, "alloc"))
    {
        uint32_t *a = malloc(sizeof(uint32_t));
        *a = 23; // dereference to check it's valid
        free(a);
        uint64_t *b = malloc(sizeof(uint64_t));
        assert((size_t)b != (size_t)a); // Check 1: won't attempt to use a free area that's too small
        free(b);                        // a & b are now free
        uint32_t *c = malloc(sizeof(uint32_t));
        assert((size_t)c == (size_t)b); // Check 2: will use a free area that's larger than the right size
        uint32_t *d = malloc(sizeof(uint32_t));
        assert((size_t)d == (size_t)a); // Check 3: will use a free area that's the right size

        void *e = malloc(sizeof(uint8_t));
        void *f = malloc(sizeof(uint8_t));
        void *g = malloc(sizeof(uint8_t));
        void *h = malloc(sizeof(uint8_t));
        void *i = malloc(sizeof(uint64_t));
        free(i);
        free(f);
        free(e);
        free(h);

        void *j = malloc(sizeof(uint32_t));
        assert((size_t)j == (size_t)i); // Check 4: will find a suitable free area that was freed several steps ago
        free(j);

        char *str = calloc(4, sizeof(char));
        strcpy(str, "LOL");
        assert(strcmp(str, "LOL") == 0);
        char *new_str = realloc(str, sizeof(char) * 2);
        assert(new_str == str);
        new_str = realloc(new_str, sizeof(char) * 9);
        assert(new_str != str);
        strcpy(new_str, "lollipop");
        assert(strcmp(new_str, "lollipop") == 0);

        printf("All tests passed!\n");
    }
    else if (!strcasecmp(curr_cmd, "breakpoint"))
    {
        printf("Triggering breakpoint:\n");
        int3();
    }
    else if (!strcasecmp(curr_cmd, "pagefault"))
    {
        printf("Triggering page fault exception:\n");
        int *ptr = (int *)0xdeadbeef;
        *ptr = 1;
    }
    else if (!strcasecmp(curr_cmd, "divbyzero"))
    {
        printf("Triggering divide by zero exception:\n");
        int i = 2;
        int j = 0;
        i /= j;
    }
    else if (!strcasecmp(curr_cmd, "lbaread"))
    {
        uint8_t *ptr = malloc(sizeof(lba_sector_t));
        uint32_t sector = 3524;
        read_sectors_ata(0, sector, 1, (lba_sector_t *)ptr);
        printf("Sector %u: ", sector);
        for (int i = 0; i < SECTOR_SIZE; i++)
            printf("%x", ptr[i]);
        printf("\n");
    }
    else if (!strcasecmp(curr_cmd, "lbawrite"))
    {
        uint8_t *ptr = malloc(sizeof(lba_sector_t));
        uint32_t sector = 3524;
        strcpy((char *)ptr, "LOL");
        write_sectors_ata(0, sector, 1, (lba_sector_t *)ptr);
    }
    else if (!strcasecmp(curr_cmd, "cpl"))
    {
        uint8_t c = cpl();
        printf("\nCurrent Protection Level: %u\n", c);
    }
    else if (!strcasecmp(curr_cmd, "open"))
    {
        FILE *file = fopen("/test.txt", "r");
        if (file == NULL)
        {
            printf("Error opening file\n");
        }
        else
        {
            int num;
            fscanf(file, "%d", &num);
            printf("found %d\n", num);
            fclose(file);
        }
    }
    else
    {
        return 0;
    }

    return 1;
}