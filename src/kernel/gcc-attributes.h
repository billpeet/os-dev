#ifndef GCC_ATTRIBUTES_H
#define GCC_ATTRIBUTES_H

// 'Naked' function - without prologue/epilogue messing up the stack pointers
#define NAKED __attribute__((naked))

// Function that never returns
#define NORETURN __attribute__((noreturn))

// Interrupt calling convention
#define INTERRUPT __attribute__((interrupt))

// Exception calling convention
#define EXCEPTION __attribute__((exception))

// Function will always be inlined where it is called
#define ALWAYS_INLINE __attribute__((always_inline))

// Packed struct - don't attempt to align
#define PACKED __attribute__((packed))

// Align struct
#define ALIGNED(align) __attribute__((aligned(align)))

#endif