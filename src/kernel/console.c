#include "console.h"
#include "types.h"
#include "stdio.h"
#include "x86.h"
#include "vga.h"
#include "kernel.h"

#define QUEUE_SIZE 100

struct queue
{
    char chars[QUEUE_SIZE];
    size_t front, rear, cnt;
} char_queue;

static int disable_counter = 0;

static void lock_queue()
{
    cli();
    disable_counter++;
}

static void unlock_queue()
{
    disable_counter--;
    if (disable_counter == 0)
        sti();
}

void init_console()
{
    lock_queue();
    char_queue.front = 0;
    char_queue.rear = 0;
    char_queue.cnt = 0;
    unlock_queue();
}

void queue(char c)
{
    if (char_queue.cnt >= QUEUE_SIZE)
        return;
    lock_queue();
    if (char_queue.rear >= QUEUE_SIZE)
        panic("Char queue is full!\n");
    char_queue.chars[char_queue.rear++] = c;
    char_queue.cnt++;
    unlock_queue();
}

char dequeue()
{
    lock_queue();
    char c = char_queue.chars[char_queue.front++];
    if (--char_queue.cnt == 0)
    {
        //Reset position to start of array
        char_queue.front = 0;
        char_queue.rear = 0;
    }
    unlock_queue();
    return c;
}

int char_available()
{
    return char_queue.cnt > 0 ? 1 : 0;
}