#include "shell.h"
#include "idt.h"
#include "stdio.h"
#include "vga.h"
#include "pong.h"
#include "alloc.h"
#include "string.h"
#include "frame_allocator.h"
#include "paging.h"
#include "kernel.h"
#include "ata.h"
#include "fat.h"
#include "task.h"
#include "x86.h"
#include <stdbool.h>
#include <stddef.h>

char curr_cmd[100];
u32 argc;
char *argv[100];
int pos;
u8 current_drive;
fat32_directory_t current_dir;
char current_dir_name[100] = "\\";
char current_dir_str[100] = "C:\\";
task_t *shell_task;
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
    if (current_dir.entries != NULL)
        free(current_dir.entries);
    current_drive = drive_number;

    init_drive(drive_number);
    current_dir = read_root_directory(drive_number);
    strcpy(current_dir_name, "\\");
    update_location_string();
}

void shell_line_init()
{
    printf(current_dir_str);
    printf("> ");
    for (int i = 0; i < 100; i++)
        curr_cmd[i] = '\0';
    argc = 0;
    pos = 0;
}

void shell_execute()
{
    putchar('\n');

    bool cmd_init = false;
    argc = 0;
    for (u16 i = 0; i < 100; i++)
    {
        if (curr_cmd[i] == ' ')
        {
            curr_cmd[i] = '\0';
            if (cmd_init)
                argv[argc++] = curr_cmd + i + 1;
            cmd_init = true;
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
        vga_clearScreen();
    }
    else if (!strcasecmp(curr_cmd, "moveup"))
    {
        vga_moveUp();
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
        u8 *ptr = malloc(sizeof(lba_sector_t));
        u32 sector = 3524;
        read_sectors_ata(0, sector, 1, (lba_sector_t *)ptr);
        printf("Sector %u: ", sector);
        for (int i = 0; i < SECTOR_SIZE; i++)
            printf("%x", ptr[i]);
        printf("\n");
    }
    else if (!strcasecmp(curr_cmd, "lbawrite"))
    {
        u8 *ptr = malloc(sizeof(lba_sector_t));
        u32 sector = 3524;
        strcpy((char *)ptr, "LOL");
        write_sectors_ata(0, sector, 1, (lba_sector_t *)ptr);
    }
    else if (!strcasecmp(curr_cmd, "dir"))
    {
        dump_directory(&current_dir);
    }
    else if (!strcasecmp(curr_cmd, "cd"))
    {
        putchar('\n');
        if (argc == 0 || argv[0][0] == '\0')
        {
            printf(current_dir_str);
            printf("\n");
        }
        else
        {
            char *file_name = argv[0];
            fat32_entry_t *entry = find_sub_directory(&current_dir, file_name);
            if (entry == NULL)
                printf("Invalid directory\n");
            else
            {
                u32 cluster_number = get_cluster_number(entry);
                if (current_dir.entries != NULL)
                    free(current_dir.entries);
                printf("Cluster number: %u\n", cluster_number);
                current_dir = read_directory(current_drive, cluster_number);
                if (strcmp(file_name, ".") != 0)
                {
                    char *c = current_dir_name;
                    if (*c != 0)
                    {
                        while (*c != '\0')
                            c++;
                    }
                    if (strcmp(file_name, "..") == 0)
                    {
                        while (c > current_dir_name + 1 && *c != '\\')
                            c--;
                        *c = '\0';
                    }
                    else
                    {
                        if (c > current_dir_name + 2)
                            *c++ = '\\';
                        strcpy(c, file_name);
                    }
                    update_location_string();
                }
            }
        }
    }
    else if (!strcasecmp(curr_cmd, "cat"))
    {
        printf("\n");
        if (argc == 0 || argv[0][0] == '\0')
        {
            printf(current_dir_str);
            printf("\n");
        }
        else
        {
            char *file_name = argv[0];
            u32 file_size;
            u8 *file_ptr = read_file(&current_dir, file_name, &file_size);
            if (file_ptr == NULL)
                printf("File not found\n");
            else
            {
                for (int i = 0; i < file_size; i++)
                    printf("%c", file_ptr[i]);
                printf("\n");
                free(file_ptr);
            }
        }
    }
    else if (!strcasecmp(curr_cmd, "touch"))
    {
        printf("\n");
        if (argc == 0 || argv[0][0] == '\0')
            printf("Usage: touch <file-name>\n");
        else
        {
            u32 file_size = 0;
            create_file(&current_dir, argv[0], "txt", file_size, 0);
        }
    }
    else if (!strcasecmp(curr_cmd, "write"))
    {
        printf("\n");
        if (argc == 0 || argv[0][0] == '\0')
            printf("Usage: write <file-name>\n");
        else
        {
            u32 file_size;
            u8 *file_ptr = read_file(&current_dir, argv[0], &file_size);
            if (file_ptr == NULL)
                printf("File %s not found\n", argv[0]);
            char *file_str = (char *)file_ptr;
            char *message = "Peanuts";
            strcpy(file_str, message);
            write_file(&current_dir, argv[0], file_ptr, strlen(message));
            free(file_ptr);
        }
    }
    else if (!strcasecmp(curr_cmd, "tskmgr"))
    {
        dump_tasks();
    }
    else if (!strcasecmp(curr_cmd, "reboot"))
    {
        printf("Rebooting...\n");
        reboot();
    }
    else if (!strcasecmp(curr_cmd, "cpl"))
    {
        u8 c = cpl();
        printf("\nCurrent Protection Level: %u\n", c);
    }
    else
    {
        // Attempt to find executable file with this name (unfinished)
        printf("\n");
        if (curr_cmd[0] != '\0' && curr_cmd[0] != ' ')
        {
            u32 file_size;
            u8 *file_ptr = read_file(&current_dir, argv[0], &file_size);
            if (file_ptr == NULL)
                printf("Unrecognised command '%s'\n", curr_cmd);
            else
            {
                // TODO: execute command
                printf("Executing %s\n", curr_cmd);
                free(file_ptr);
            }
        }
    }
    shell_line_init();
}

void shell(void)
{
    shell_task = running_task;
    current_dir.entries = NULL;
    change_drive(0);

    shell_line_init();

    while (1)
    {
        char c = getchar();
        if (c == '\b')
        {
            if (pos > 0)
            {
                curr_cmd[--pos] = '\0';
                putchar(c);
            }
        }
        else if (c == '\n')
        {
            shell_execute();
        }
        else
        {
            putchar(c);
            curr_cmd[pos++] = c;
        }
    }
}