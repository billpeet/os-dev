#ifndef MEMORY_H
#define MEMORY_H

void init_paging();
void *get_physaddr(void *virt_addr);

#endif