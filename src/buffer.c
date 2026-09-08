/*
 * Implementation of the bounded circular buffer.
 *
 * Responsibilities:
 * - Apply the producer/consumer protocol of the active sync_mode.
 * - Keep the counting semaphores (capacity) separate from the binary
 *   semaphore (mutual exclusion), so a run can drop one without the other.
 */

#include "buffer.h"

#include <semaphore.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* NONE is the only mode without the capacity semaphores empty and full. */
static bool uses_counting_semaphores(SyncMode sync_mode)
{
    return sync_mode != SYNC_MODE_NONE;
}

/* FULL is the only mode with mutual exclusion over the indices. */
static bool uses_mutex(SyncMode sync_mode)
{
    return sync_mode == SYNC_MODE_FULL;
}

/*
 * A signal makes sem_wait return -1 with EINTR without decrementing. Retrying
 * keeps the capacity and mutual-exclusion invariants of FULL from breaking for
 * a reason the experiment is not measuring.
 */
static void sem_wait_retrying(sem_t *semaphore)
{
    while (sem_wait(semaphore) != 0) {
        /* EINTR is the only failure an initialised semaphore can report. */
    }
}

int buffer_init(Buffer *buffer, SyncMode sync_mode)
{
    /*
     * The slots start defined so that a NONE run reading a never-written slot
     * yields a wrong value, not undefined behaviour.
     */
    memset(buffer->slots, 0, sizeof buffer->slots);
    buffer->write_index = 0;
    buffer->read_index = 0;
    buffer->sync_mode = sync_mode;

    /*
     * Only the semaphores the mode uses are created. NONE omits all three and
     * NO_MUTEX omits the binary one on purpose: each mode must remove exactly
     * one thing, so that the checksum divergence has a single explanation.
     */
    if (uses_counting_semaphores(sync_mode)) {
        if (sem_init(&buffer->empty, 0, BUFFER_SIZE) != 0) {
            return -1;
        }
        if (sem_init(&buffer->full, 0, 0) != 0) {
            return -1;
        }
    }
    if (uses_mutex(sync_mode) && sem_init(&buffer->mutex, 0, 1) != 0) {
        return -1;
    }
    return 0;
}

void buffer_put(Buffer *buffer, int item)
{
    /* Counter before mutex, always: the inverse order deadlocks. */
    if (uses_counting_semaphores(buffer->sync_mode)) {
        sem_wait_retrying(&buffer->empty);
    }
    /*
     * Under NONE nothing above waits for a free slot, and under NONE or
     * NO_MUTEX nothing below serialises the write. Both omissions are
     * intentional: they are the variable the experiment manipulates.
     */
    if (uses_mutex(buffer->sync_mode)) {
        sem_wait_retrying(&buffer->mutex);
    }

    buffer->slots[buffer->write_index] = item;
    buffer->write_index = (buffer->write_index + 1) % BUFFER_SIZE;

    if (uses_mutex(buffer->sync_mode)) {
        sem_post(&buffer->mutex);
    }
    if (uses_counting_semaphores(buffer->sync_mode)) {
        sem_post(&buffer->full);
    }
}

int buffer_take(Buffer *buffer)
{
    int item;

    if (uses_counting_semaphores(buffer->sync_mode)) {
        sem_wait_retrying(&buffer->full);
    }
    /*
     * Same intentional omissions as in buffer_put. Under NONE the consumer
     * does not block on an empty buffer: it reads the slot as it stands and
     * the stale or still-zero value surfaces in the checksum. The run must
     * end by quota in every mode — the failure is a wrong number, never a hang.
     */
    if (uses_mutex(buffer->sync_mode)) {
        sem_wait_retrying(&buffer->mutex);
    }

    item = buffer->slots[buffer->read_index];
    buffer->read_index = (buffer->read_index + 1) % BUFFER_SIZE;

    if (uses_mutex(buffer->sync_mode)) {
        sem_post(&buffer->mutex);
    }
    if (uses_counting_semaphores(buffer->sync_mode)) {
        sem_post(&buffer->empty);
    }

    return item;
}

void buffer_destroy(Buffer *buffer)
{
    /* Only the semaphores buffer_init created are destroyed. */
    if (uses_counting_semaphores(buffer->sync_mode)) {
        sem_destroy(&buffer->empty);
        sem_destroy(&buffer->full);
    }
    if (uses_mutex(buffer->sync_mode)) {
        sem_destroy(&buffer->mutex);
    }
}
