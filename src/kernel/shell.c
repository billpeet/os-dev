#include "shell.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "idt.h"
#include "frame_allocator.h"
#include "paging.h"
#include "kernel.h"
#include "fat.h"
#include "task.h"
#include "x86.h"
#include "system/test_commands.h"

char curr_cmd[100];
uint32_t argc;
char *argv[100];
int pos;
fat32_directory_t current_dir;
task_t *shell_task;
bool cmd_in_progress;

void shell_line_init()
{
    printf(current_dir.path);
    printf("$ ");
    for (int i = 0; i < 100; i++)
        curr_cmd[i] = '\0';
    argc = 0;
    pos = 0;
}

void shell_execute()
{
    putchar('\n');

    argc = 0;
    for (uint16_t i = 0; i < 100; i++)
    {
        if (curr_cmd[i] == ' ')
        {
            // printf("space detected\n");
            curr_cmd[i] = '\0';
            // printf("arg %u: %c\n", argc, *(curr_cmd + i + 1));
            argv[argc++] = curr_cmd + i + 1;
        }
    }

    if (try_run_test_cmd(curr_cmd))
    {
        // ran test command
    }
    else if (!strcasecmp(curr_cmd, "clear"))
    {
        vga_clearScreen();
    }
    else if (!strcasecmp(curr_cmd, "moveup"))
    {
        vga_moveUp();
    }
    else if (!strcasecmp(curr_cmd, "dir"))
    {
        dump_directory_windows(&current_dir);
    }
    else if (!strcasecmp(curr_cmd, "ls"))
    {
        dump_directory_unix(&current_dir);
    }
    else if (!strcasecmp(curr_cmd, "cd"))
    {
        // putchar('\n');
        // if (argc == 0 || argv[0][0] == '\0')
        // {
        //     printf(current_dir.path);
        //     printf("\n");
        // }
        // else
        // {
        //     char *dir_name = argv[0];
        //     fat32_directory_t new_dir;
        //     if (load_sub_directory(&current_dir, dir_name, &new_dir) != 0)
        //         printf("Invalid directory '%s'\n", dir_name);
        //     else
        //     {
        //         // Success - free old directory
        //         if (strcmp(current_dir.path, new_dir.path) != 0 && strlen(current_dir.path) > 3)
        //             free(current_dir.entries);
        //         current_dir = new_dir;
        //     }
        // }
    }
    else if (!strcasecmp(curr_cmd, "cat"))
    {
        if (argc == 0 || argv[0][0] == '\0')
        {
            printf(current_dir.path);
            printf("\n");
        }
        else
        {
            char *file_name = argv[0];
            uint32_t *fat = (uint32_t *)NULL;
            fat32_entry_t *entry = malloc(sizeof(fat32_entry_t));
            if (load_entry(&current_dir, fat, file_name, entry) < 0)
            {
                printf("file not found\n");
            }
            else
            {
                // dump_fat32_entry_verbose(entry);
                char contents[100] = {0};
                int bytes_read = read_file(entry, fat, current_dir.drive_number, contents, 100);
                // printf("read %u bytes\n", bytes_read);
                for (size_t i = 0; i < bytes_read; i++)
                    printf("%c", contents[i]);
                printf("\n");
            }
            free(entry);
        }
    }
    else if (!strcasecmp(curr_cmd, "touch"))
    {
        printf("\n");
        if (argc == 0 || argv[0][0] == '\0')
            printf("Usage: touch <file-name>\n");
        else
        {
            uint32_t file_size = 0;
            create_file(&current_dir, argv[0], "txt", file_size, 0);
        }
    }
    else if (!strcasecmp(curr_cmd, "write"))
    {
        // printf("\n");
        // if (argc == 0 || argv[0][0] == '\0')
        //     printf("Usage: write <file-name>\n");
        // else
        // {
        //     uint32_t file_size;
        //     uint8_t *file_ptr = read_file(&current_dir, argv[0], &file_size);
        //     if (file_ptr == NULL)
        //         printf("File %s not found\n", argv[0]);
        //     char *file_str = (char *)file_ptr;
        //     char *message = "Peanuts";
        //     strcpy(file_str, message);
        //     write_file(&current_dir, argv[0], file_ptr, strlen(message));
        //     free(file_ptr);
        // }
    }
    else if (!strcasecmp(curr_cmd, "ren"))
    {
        // Rename file
        printf("\n");
        if (argc <= 1)
            printf("Usage: ren <filename> <filename> (%u args passed)\n", argc);
        else
        {
            if (rename(argv[0], argv[1]) == 0)
                printf("Successfully renamed '%s' to '%s'\n", argv[0], argv[1]);
            else
                printf("Could not rename '%s' to '%s'\n", argv[0], argv[1]);
        }
    }
    else if (!strcasecmp(curr_cmd, "tskmgr"))
    {
        // Task manager - shows currently running tasks
        dump_tasks();
    }
    else if (!strcasecmp(curr_cmd, "reboot"))
    {
        printf("Rebooting...\n");
        reboot();
    }
    else
    {
        // Attempt to find executable file with this name (unfinished)
        printf("\n");
        // if (curr_cmd[0] != '\0' && curr_cmd[0] != ' ')
        // {
        //     uint32_t file_size;
        //     uint8_t *file_ptr = read_file(&current_dir, argv[0], &file_size);
        //     if (file_ptr == NULL)
        //         printf("Unrecognised command '%s'\n", curr_cmd);
        //     else
        //     {
        //         // TODO: execute command
        //         printf("Executing %s\n", curr_cmd);
        //         free(file_ptr);
        //     }
        // }
    }
    shell_line_init();
}

void shell(void)
{
    shell_task = running_task;
    current_dir.entries = NULL;

    // load root directory
    read_directory(0, 0, "/", &current_dir);

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