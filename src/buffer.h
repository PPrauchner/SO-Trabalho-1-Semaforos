/*
 * Bounded circular buffer shared between producer and consumer threads.
 *
 * Responsibilities:
 * - Own the slots, the read/write indices and the three POSIX semaphores.
 * - Decide, from the sync_mode fixed at initialisation, which semaphores guard
 *   each access.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <semaphore.h>
#include <stddef.h>

/* Number of slots in the circular buffer. */
#define BUFFER_SIZE 8

/* Which set of semaphores is active in a given run. */
typedef enum {
    SYNC_MODE_FULL,
} SyncMode;

/* The shared resource whose corruption the experiment measures. */
typedef struct {
    int slots[BUFFER_SIZE];
    size_t write_index;
    size_t read_index;
    sem_t empty;   /* counts free slots */
    sem_t full;    /* counts stored items */
    sem_t mutex;   /* binary semaphore guarding the indices */
    SyncMode sync_mode;
} Buffer;

/*
 * Prepares an empty buffer for the given synchronisation mode.
 *
 * buffer:    buffer to initialise.
 * sync_mode: mode that stays fixed for the whole run.
 *
 * Returns 0 on success, -1 if a semaphore could not be initialised.
 */
int buffer_init(Buffer *buffer, SyncMode sync_mode);

/*
 * Stores an item, blocking while the buffer is full.
 *
 * buffer: target buffer, already initialised.
 * item:   value to store.
 */
void buffer_put(Buffer *buffer, int item);

/*
 * Removes an item, blocking while the buffer is empty.
 *
 * buffer: source buffer, already initialised.
 *
 * Returns the item read from the buffer.
 */
int buffer_take(Buffer *buffer);

/* Releases the semaphores owned by the buffer. */
void buffer_destroy(Buffer *buffer);

#endif /* BUFFER_H */
