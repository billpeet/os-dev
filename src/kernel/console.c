#include "console.h"
#include "types.h"

struct queue
{
    char chars[100];
    signed short curr;
} char_queue;

int lockcnt = 0;

void lock_queue()
{
    while (lockcnt > 0)
        ;
    lockcnt++;
}

void unlock()
{
    lockcnt--;
}

void init_console()
{
    char_queue.curr = -1;
    printf("curr after init: %i\n", (long)char_queue.curr);
}

void queue(char c)
{
    lock_queue();
    printf("queueing 0x%x at %i\n", c, char_queue.curr + 1);
    char_queue.chars[++char_queue.curr] = c;
    unlock();
}

char dequeue()
{
    lock_queue();
    printf("dequeuing 0x%x\n", char_queue.chars[char_queue.curr]);
    return char_queue.chars[char_queue.curr--];
    unlock();
}

int char_available()
{
    printf("curr: %i\n", (long)char_queue.curr);
    return char_queue.curr >= 0 ? 1 : 0;
}