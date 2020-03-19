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

char curr_cmd[100];
char curr_param[100];
int pos;
u8 current_drive;
fat32_entry_t *current_dir;
char current_dir_name[100] = "\\";
char current_dir_str[100] = "C:\\";

void shell_char(char c);

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
    current_drive = drive_number;
    current_dir = read_root_directory(drive_number);
    strcpy(current_dir_name, "\\");
    update_location_string();
}

void shell_line_init()
{
    writeString(current_dir_str);
    writeString("> ");
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
        writeString("Starting pong:\n");
        pong();
        writeString("done with pong\n");
        register_handler(shell_char);
    }
    else if (!strcasecmp(curr_cmd, "clear"))
    {
        clearScreen();
    }
    else if (!strcasecmp(curr_cmd, "newframe"))
    {
        void *new_frame = allocate_frame();
        writeStrHexInt("New frame: ", (u64)new_frame);
        // for (u64 i = 0;; i++)
        // {
        //     if (allocate_frame() == 0 && i != 0)
        //     {
        //         writeString("Allocated ");
        //         writeInt(i);
        //         writeString(" frames\n");
        //         break;
        //     }
        // }
    }
    else if (!strcasecmp(curr_cmd, "deaframe"))
    {
        void *new_frame1 = allocate_frame();
        void *new_frame2 = allocate_frame();
        void *new_frame3 = allocate_frame();
        writeStrHexInt("New frame: ", (u64)new_frame3);
        writeString("Now deallocating again\n");
        deallocate_frame(new_frame3);
        deallocate_frame(new_frame2);
        deallocate_frame(new_frame1);
        void *new_new_frame = allocate_frame();
        writeStrHexInt("Next frame: ", (u64)new_new_frame);
    }
    else if (!strcasecmp(curr_cmd, "paging"))
    {
        u64 ptr = 0x00000fffffeee010;
        ptr = 0x20003;
        u64 phys_addr = get_physaddr((void *)ptr);
        writeStrHexInt("Phys_addr: ", (u64)phys_addr);
    }
    else if (!strcasecmp(curr_cmd, "recursive"))
    {
        u64 l4_table_addr = (u64)level_4_table();
        page_table_t *p4 = (page_table_t *)0xfffffffffffff000;
        u64 phys_addr = get_physaddr(p4);
        writeString("P4 table:");
        writeHexInt((u64)p4);
        writeString(", phys_addr:");
        writeHexInt((u64)phys_addr);
        writeString(", actual:");
        writeHexInt((u64)l4_table_addr);
        writeNewLine();
        writeString("Attempting to deallocate: ");
        writeHexInt(p4->entries[0]);
        writeNewLine();
    }
    else if (!strcasecmp(curr_cmd, "newpage"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        writeString("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        writeString("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        writeString("Phys_addr:");
        writeHexInt((u64)phys_addr);
        writeNewLine();
    }
    else if (!strcasecmp(curr_cmd, "newhuge"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        writeString("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10000010); // huge
        writeString("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        writeString("Phys_addr:");
        writeHexInt((u64)phys_addr);
        writeNewLine();
    }
    else if (!strcasecmp(curr_cmd, "unmap"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        writeString("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        writeString("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        writeString("Phys_addr:");
        writeHexInt((u64)phys_addr);
        writeNewLine();
        int *int_ptr = (int *)ptr;
        writeString("Here's its value:");
        writeInt(*int_ptr);
        unmap_page((void *)ptr);
        writeString("\nUnampped, now attempting to get physical addr:\n");
        get_physaddr((void *)ptr);
        writeInt(*int_ptr);
    }
    else if (!strcasecmp(curr_cmd, "alloc"))
    {
        u32 *a = malloc(sizeof(u32));
        *a = 23; // dereference to check it's valid
        free(a);
        u64 *b = malloc(sizeof(u64));
        if ((u64)b == (u64)a)
            writeString("ASSERT 1 FAILED\n"); // Check 1: won't attempt to use a free area that's too small
        free(b);                              // a & b are now free
        u32 *c = malloc(sizeof(u32));
        if ((u64)c != (u64)b)
            writeString("ASSERT 2 FAILED\n"); // Check 2: will use a free area that's larger than the right size
        u32 *d = malloc(sizeof(u32));
        if ((u64)d != (u64)a)
            writeString("ASSERT 3 FAILED\n"); // Check 3: will use a free area that's the right size

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
            writeString("ASSERT 4 FAILED\n"); // Check 4: will find a suitable free area that was freed several steps ago

        writeString("All tests passed!\n");
    }
    else if (!strcasecmp(curr_cmd, "breakpoint"))
    {
        writeString("Triggering breakpoint exception:\n");
        asm("int3");
    }
    else if (!strcasecmp(curr_cmd, "pagefault"))
    {
        writeString("Triggering page fault exception:\n");
        int *ptr = (int *)0xdeadbeef;
        *ptr = 1;
    }
    else if (!strcasecmp(curr_cmd, "divbyzero"))
    {
        writeString("Triggering divide by zero exception:\n");
        int i = 0;
        i /= i;
    }
    else if (!strcasecmp(curr_cmd, "lba"))
    {
        u8 *ptr = allocate_frame();
        read_sectors_lba(0, 1, 1, (lba_sector_t *)ptr);
        writeString("Before: ");
        for (int i = 0; i < SECTOR_SIZE; i++)
        {
            writeHexInt(ptr[i]);
        }
        write_sectors_lba(0, 1, 1, (lba_sector_t *)ptr);
        u8 *ptr2 = allocate_frame();
        read_sectors_lba(0, 1, 1, (lba_sector_t *)ptr2);
        writeString("\nAfter: ");
        for (int i = 0; i < SECTOR_SIZE; i++)
        {
            writeHexInt(ptr2[i]);
        }
        writeNewLine();
    }
    else if (!strcasecmp(curr_cmd, "dir"))
    {
        dump_directory(current_dir);
    }
    else if (!strcasecmp(curr_cmd, "cd"))
    {
        writeNewLine();
        if (curr_param[0] == '\0' || curr_param[0] == ' ')
        {
            writeString(current_dir_str);
            writeNewLine();
        }
        else
        {
            fat32_entry_t *entry = find_sub_directory(current_dir, curr_param);
            if (entry == NULLPTR)
                writeString("Invalid directory\n");
            else
            {
                u32 cluster_number = get_cluster_number(entry);
                free(current_dir);
                writeStrInt("Cluster number: ", cluster_number);
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
            writeString(current_dir_str);
            writeNewLine();
        }
        else
        {
            u32 file_size;
            u8 *file_ptr = read_file(current_drive, current_dir, curr_param, &file_size);
            if (file_ptr == NULLPTR)
                writeString("File not found\n");
            else
            {
                writeStrInt("File size ", file_size);
                for (int i = 0; i < file_size && i < 255; i++)
                {
                    writeChar(file_ptr[i]);
                }
                writeNewLine();
                free(file_ptr);
            }
        }
    }
    else if (!strcasecmp(curr_cmd, "reboot"))
    {
        writeString("Rebooting...\n");
        reboot();
    }
    else
    {
        writeString("Unrecognised command '");
        writeString(curr_cmd);
        writeString("'\n");
    }
    shell_line_init();
}

void shell_char(char c)
{
    if (c == '\b')
    {
        if (pos > 0)
        {
            curr_cmd[--pos] = '\0';
            writeChar(c);
        }
    }
    else if (c == '\n')
        shell_execute();
    else
    {
        writeChar(c);
        curr_cmd[pos++] = c;
    }
}

void shell(void)
{
    change_drive(0);

    shell_line_init();
    register_handler(shell_char);

    while (1)
        asm("hlt");
}