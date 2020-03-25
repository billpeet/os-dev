#include "shell.h"
#include "idt.h"
#include "vga.h"
#include "pong.h"
#include "alloc.h"
#include "string.h"
#include "frame_allocator.h"
#include "paging.h"
#include "kernel.h"
#include "lba.h"
#include "fat.h"
#include "task.h"
#include "x86.h"
#include <stdbool.h>

int_handler_t handler;
char curr_cmd[100];
char curr_param[100];
int pos;
u8 current_drive;
fat32_directory_t current_dir;
char current_dir_name[100] = "\\";
char current_dir_str[100] = "C:\\";
task_t shell_task;
bool cmd_in_progress;

void shell_char();

int starts_with(char *input, char const *check)
{
    char c;
    for (int i = 0; c = check[i]; i++)
    {
        if (!input[i] || input[i] != check[i])
            return 0;
    }
    return 1;
}

void update_location_string()
{
    current_dir_str[0] = current_drive + 67;
    current_dir_str[1] = ':';

    u16 i;
    for (i = 0; current_dir_name[i] != '\0'; i++)
    {
        current_dir_str[i + 2] = current_dir_name[i];
    }
    current_dir_str[i + 2] = '\0';
}

void change_drive(u8 drive_number)
{
    if (current_dir.entries != NULLPTR)
        free(current_dir.entries);
    current_drive = drive_number;
    current_dir = read_root_directory(drive_number);
    strcpy(current_dir_name, "\\");
    update_location_string();
}

void shell_line_init()
{
    printf(current_dir_str);
    printf("> ");
    for (int i = 0; i < 100; i++)
    {
        curr_cmd[i] = '\0';
        curr_param[i] = '\0';
    }
    pos = 0;
}

void shell_execute()
{
    writeChar('\n');

    for (u16 i = 0; i < 100; i++)
    {
        if (curr_cmd[i] == ' ')
        {
            curr_cmd[i] = '\0';
            for (u16 j = 0; j < 100 - i - 1; j++)
            {
                curr_param[j] = curr_cmd[i + j + 1];
                curr_cmd[i + j + 1] = '\0';
            }
            break;
        }
    }

    if (!strcasecmp(curr_cmd, "pong"))
    {
        printf("Starting pong:\n");
        // unregister_handler(handler);
        cmd_in_progress = true;
        pong();
        cmd_in_progress = false;
        printf("done with pong\n");
        // register_handler(shell_char);
    }
    else if (!strcasecmp(curr_cmd, "clear"))
    {
        clearScreen();
    }
    else if (!strcasecmp(curr_cmd, "newframe"))
    {
        void *new_frame = allocate_frame();
        printf("New frame: 0x%p\n", new_frame);
        // for (u64 i = 0;; i++) // allocate all frames
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
        u64 ptr = 0x00000fffffeee010;
        ptr = 0x20003;
        u64 phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: 0x%p\n", phys_addr);
    }
    else if (!strcasecmp(curr_cmd, "recursive"))
    {
        u64 l4_table_addr = (u64)level_4_table();
        page_table_t *p4 = (page_table_t *)0xfffffffffffff000;
        u64 phys_addr = get_physaddr(p4);
        printf("P4 table: 0x%p, phys_addr:0x%x, actual: 0x%u\n", p4, phys_addr, l4_table_addr);
        printf("Attempting to deallocate: %x\n", p4->entries[0]);
    }
    else if (!strcasecmp(curr_cmd, "newpage"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        printf("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        printf("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: %p\n", phys_addr);
    }
    else if (!strcasecmp(curr_cmd, "newhuge"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        printf("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10000010); // huge
        printf("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: %p\n", phys_addr);
    }
    else if (!strcasecmp(curr_cmd, "unmap"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        printf("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        printf("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        printf("Phys_addr: %p\n", phys_addr);
        int *int_ptr = (int *)ptr;
        printf("Here's its value: %i\n", int_ptr);
        unmap_page((void *)ptr);
        printf("Unampped, now attempting to get physical addr:\n");
        get_physaddr((void *)ptr);
        printf("%i\n", *int_ptr);
    }
    else if (!strcasecmp(curr_cmd, "alloc"))
    {
        u32 *a = malloc(sizeof(u32));
        *a = 23; // dereference to check it's valid
        free(a);
        u64 *b = malloc(sizeof(u64));
        if ((u64)b == (u64)a)
            printf("ASSERT 1 FAILED\n"); // Check 1: won't attempt to use a free area that's too small
        free(b);                         // a & b are now free
        u32 *c = malloc(sizeof(u32));
        if ((u64)c != (u64)b)
            printf("ASSERT 2 FAILED\n"); // Check 2: will use a free area that's larger than the right size
        u32 *d = malloc(sizeof(u32));
        if ((u64)d != (u64)a)
            printf("ASSERT 3 FAILED\n"); // Check 3: will use a free area that's the right size

        void *e = malloc(sizeof(u8));
        void *f = malloc(sizeof(u8));
        void *g = malloc(sizeof(u8));
        void *h = malloc(sizeof(u8));
        void *i = malloc(sizeof(u64));
        free(i);
        free(f);
        free(e);
        free(h);

        void *j = malloc(sizeof(u32));
        if ((u64)j != (u64)i)
            printf("ASSERT 4 FAILED\n"); // Check 4: will find a suitable free area that was freed several steps ago

        printf("All tests passed!\n");
    }
    else if (!strcasecmp(curr_cmd, "breakpoint"))
    {
        printf("Triggering breakpoint exception:\n");
        asm("int3");
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
        int i = 0;
        i /= i;
    }
    else if (!strcasecmp(curr_cmd, "lba"))
    {
        u8 *ptr = malloc(sizeof(lba_sector_t));
        u8 *ptr2 = malloc(sizeof(lba_sector_t));
        u32 sector = 0;
        read_sectors_lba(0, sector, 1, (lba_sector_t *)ptr);
        read_sectors_lba(0, sector, 1, (lba_sector_t *)ptr2);
        printf("Before 1: ");
        for (int i = 0; i < SECTOR_SIZE; i++)
        {
            printf("%x", ptr[i]);
        }
        printf("\nBefore 2: ");
        for (int i = 0; i < SECTOR_SIZE; i++)
        {
            printf("%x", ptr2[i]);
        }
        printf("\n");

        // ptr[0] = 0xaa;
        // ptr[1] = 0xbb;
        // ptr[2] = 0xcc;
        // ptr[3] = 0xdd;

        // write_sectors_lba(0, sector, 1, (lba_sector_t *)ptr);
        // ptr[0] = 0x00;
        // read_sectors_lba(0, sector, 1, (lba_sector_t *)ptr);
        // printf("\nAfter: ");
        // for (int i = 0; i < SECTOR_SIZE; i++)
        // {
        //     printf("%x", ptr[i]);
        // }
        // writeChar('\n');
    }
    else if (!strcasecmp(curr_cmd, "dir"))
    {
        dump_directory(current_dir);
    }
    else if (!strcasecmp(curr_cmd, "cd"))
    {
        writeChar('\n');
        if (curr_param[0] == '\0' || curr_param[0] == ' ')
        {
            printf(current_dir_str);
            writeNewLine();
        }
        else
        {
            fat32_entry_t *entry = find_sub_directory(current_dir, curr_param);
            if (entry == NULLPTR)
                printf("Invalid directory\n");
            else
            {
                u32 cluster_number = get_cluster_number(entry);
                if (current_dir.entries != NULLPTR)
                    free(current_dir.entries);
                printf("Cluster number: %u\n", cluster_number);
                current_dir = read_directory(current_drive, cluster_number);
                if (strcmp(curr_param, ".") != 0)
                {
                    char *c = current_dir_name;
                    if (*c != 0)
                    {
                        while (*c != '\0')
                            c++;
                    }
                    if (strcmp(curr_param, "..") == 0)
                    {
                        while (c > current_dir_name + 1 && *c != '\\')
                            c--;
                        *c = '\0';
                    }
                    else
                    {
                        if (c > current_dir_name + 2)
                            *c++ = '\\';
                        strcpy(c, curr_param);
                    }
                    update_location_string();
                }
            }
        }
    }
    else if (!strcasecmp(curr_cmd, "cat"))
    {
        writeNewLine();
        if (curr_param[0] == '\0' || curr_param[0] == ' ')
        {
            printf(current_dir_str);
            writeNewLine();
        }
        else
        {
            u32 file_size;
            u8 *file_ptr = read_file(current_drive, current_dir, curr_param, &file_size);
            if (file_ptr == NULLPTR)
                printf("File not found\n");
            else
            {
                printf("File size %u\n", file_size);
                for (int i = 0; i < file_size /*&& i < 100*/; i++)
                    printf("%i", file_ptr[i]);
                writeNewLine();
                free(file_ptr);
            }
        }
    }
    else if (!strcasecmp(curr_cmd, "touch"))
    {
        writeNewLine();
        if (curr_param[0] == '\0' || curr_param[0] == ' ')
        {
            printf("No file provided\n");
        }
        else
        {
            u32 file_size = 2;
            printf("Adding file %s\n", curr_param);
            create_file(current_drive, current_dir, curr_param, ".txt", file_size);
            printf("Added file %s\n", curr_param);
        }
    }
    else if (!strcasecmp(curr_cmd, "reboot"))
    {
        printf("Rebooting...\n");
        reboot();
    }
    else
    {
        printf("Unrecognised command '");
        printf(curr_cmd);
        printf("'\n");
    }
    shell_line_init();
}

void shell_char()
{
    // u64 rsp;
    // asm volatile("mov %%rsp, %%rax\n\tmov %%rax, %0"
    //              : "=m"(rsp)::"rax");
    // printf("rsp in shell_char: %x\n", rsp);
    // if (!cmd_in_progress)
    // {

    //     char c = last_char;
    //     if (c == '\b')
    //     {
    //         if (pos > 0)
    //         {
    //             curr_cmd[--pos] = '\0';
    //             writeChar(c);
    //         }
    //     }
    //     else if (c == '\n')
    //         shell_execute();
    //     else
    //     {
    //         writeChar(c);
    //         curr_cmd[pos++] = c;
    //     }
    // }

    // printf("Yielding back from shell\n");
    // printf("value at 0x44ffc8: 0x%x\n", *((u64 *)0x44ffc8));
    printf("value at 0x44ffc8: 0x%x\n", 0);

    yield();
    hlt();
}

void shell(void)
{
    change_drive(0);

    shell_line_init();
    handler.handler = shell_char;
    handler.task = &shell_task;
    register_kbhandler(handler);
    // printf("registered handler\n");

    u64 i = 0;
    // yield();
    while (1)
    {
        // yield();
        // printf("y");
        asm("hlt");
    }
}