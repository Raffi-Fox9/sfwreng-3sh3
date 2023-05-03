/**
 * @file         assignment2.c
 * @description  3SH3 assignment 2 - memory management
 * @author       Yu Liu, Raffi Fox
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#define PAGE_NUMBER_BITS 8U
#define PAGE_OFFSET_BITS 8U

#define PAGE_SIZE (1U << PAGE_OFFSET_BITS)
#define MAX_PAGES (1U << PAGE_NUMBER_BITS)

#define FRAME_NUMBER_BITS 7U
#define MAX_FRAMES (1U << FRAME_NUMBER_BITS)

#define PAGE_NUMBER(logical_addr) \
    ((logical_addr >> PAGE_OFFSET_BITS) & (MAX_PAGES - 1U))
#define PAGE_OFFSET(logical_addr) \
    (logical_addr & (PAGE_SIZE - 1U))

#define TLB_MAX_ENTRIES 16U

#define BACKING_STORE_SIZE (MAX_PAGES * PAGE_SIZE)
#define PHYSICAL_MEMORY_SIZE (MAX_FRAMES * PAGE_SIZE)

typedef struct {
    int page_number;
    int frame_number;
} TLBentry;

typedef struct {
    TLBentry entries[TLB_MAX_ENTRIES];
    int head;
    int tail;
} TLB;

int search_TLB(TLB *tlb, int page) {
    int frame = -1;
    for (int i=0; i<TLB_MAX_ENTRIES; ++i) {
        if (tlb->entries[i].page_number == page) {
            frame = tlb->entries[i].frame_number;
            break;
        }
    }
    return frame;
}

void TLB_Add(TLB *tlb, int page, int frame)
{
    // Add the entry to the front of the list
    tlb->entries[tlb->head].page_number = page;
    tlb->entries[tlb->head].frame_number = frame;
    tlb->head = (tlb->head + 1) % TLB_MAX_ENTRIES;

    // Replace the oldest entry when TLB is full
    if (tlb->head == tlb->tail) {
        tlb->tail = (tlb->tail + 1) % TLB_MAX_ENTRIES;
    }
}

int TLB_Update(TLB *tlb, int page, int old_frame)
{
    // Replace the page number corresponding to a frame
    int updated = 0;
    for (int i=0; i<TLB_MAX_ENTRIES; ++i) {
        if (tlb->entries[i].frame_number == old_frame) {
            tlb->entries[i].page_number = page;
            updated = 1;
            break;
        }
    }
    // Returns whether a TLB entry was actually updated
    return updated;
}

// Keep track of the page numbers currently in memory in the 
// order that they were requested. -1 if frame is never used
typedef struct {
    int page_ref[MAX_FRAMES];
    int head;
    int tail;
} PageFIFO;

PageFIFO *FIFO_Create() {
    PageFIFO *fifo = malloc(sizeof(PageFIFO));
    fifo->head = 0;
    fifo->tail = 0;

    // Mark all frames as unoccupied initially
    for (int i=0; i<MAX_FRAMES; ++i) {
        fifo->page_ref[i] = -1;
    }
    return fifo;
}

void FIFO_Add(PageFIFO *fifo, int new_page) {
    // Add the new page to the front of the list
    fifo->page_ref[fifo->head] = new_page;
    fifo->head = (fifo->head + 1) % MAX_FRAMES;

    // Replace the oldest entry when the FIFO is full
    if (fifo->head == fifo->tail) {
        fifo->tail = (fifo->tail + 1) % MAX_FRAMES;
    }
}

int main(void) {

    // Initialize page table to -1
    int *page_table = malloc(MAX_PAGES * sizeof(int));
    for (unsigned int i=0U; i<MAX_PAGES; ++i) {
        page_table[i] = -1;
    }

    // Initialize TLB
    TLB *tlb = calloc(1, sizeof(TLB));

    // Initialize the backing store
    uint8_t *backing_store;
    int fd = open("./BACKING_STORE.bin", O_RDONLY);
    backing_store = mmap(NULL, BACKING_STORE_SIZE, PROT_WRITE, MAP_PRIVATE, fd, 0);
    
    // Initialize statistics
    int total_addresses = 0;
    int page_faults = 0;
    int TLB_hits = 0;

    // Initialize physical memory
    uint8_t *physical_memory = malloc(PHYSICAL_MEMORY_SIZE);
    PageFIFO *fifo = FIFO_Create();

    // Simulate the MMU using addresses.txt
    FILE * f_addrs;
    int logical_addr, physical_addr, page, offset, frame, value;
    TLBentry *entry;

    f_addrs = fopen("./addresses.txt", "r");
    while (fscanf(f_addrs, "%d", &logical_addr) > 0) {
        
        page = PAGE_NUMBER(logical_addr);
        offset = PAGE_OFFSET(logical_addr);
        total_addresses += 1;

        // TLB lookup
        frame = search_TLB(tlb, page);
        if (frame != -1) {
            TLB_hits += 1;
        }
        else {
            // TLB miss
            int TLB_updated = 0;
            frame = page_table[page];
            if (frame == -1) {
                // Page Fault
                page_faults += 1;
                
                // Get empty frame
                frame = fifo->head;

                // Swap out the oldest page (if the fifo is full)
                int oldest_page = fifo->page_ref[frame];
                if (oldest_page != -1) {
                    int old_frame = page_table[oldest_page];
                    page_table[oldest_page] = -1;
                    TLB_updated = TLB_Update(tlb, page, old_frame);
                }

                // Add/replace the page in the FIFO
                FIFO_Add(fifo, page);

                // Copy an entire page to physical memory
                memcpy(&physical_memory[frame * PAGE_SIZE], 
                        &backing_store[page * PAGE_SIZE], PAGE_SIZE);

                // Update the new page table
                page_table[page] = frame;
            }

            // Cache the page-frame pair into the TLB
            if (!TLB_updated) {
                TLB_Add(tlb, page, frame);
            }
        }

        physical_addr = frame << PAGE_OFFSET_BITS | offset;
        value = (int8_t) physical_memory[physical_addr];

        printf("Virtual address: %d Physical address = %d Value=%d\n", 
                logical_addr, physical_addr, value);
    }
    fclose(f_addrs);
    munmap(backing_store, BACKING_STORE_SIZE);
    close(fd);
    free(fifo);
    free(physical_memory);
    free(tlb);
    free(page_table);

    printf("Total addresses = %d\n", total_addresses);
    printf("Page_faults = %d\n", page_faults);
    printf("TLB Hits = %d\n", TLB_hits);
    return 0;
}