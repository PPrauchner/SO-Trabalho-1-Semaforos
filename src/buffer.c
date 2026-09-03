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
#include <stddef.h>

int buffer_init(Buffer *buffer, SyncMode sync_mode)
{
    buffer->write_index = 0;
    buffer->read_index = 0;
    buffer->sync_mode = sync_mode;

    if (sem_init(&buffer->empty, 0, BUFFER_SIZE) != 0) {
        return -1;
    }
    if (sem_init(&buffer->full, 0, 0) != 0) {
        return -1;
    }
    if (sem_init(&buffer->mutex, 0, 1) != 0) {
        return -1;
    }
    return 0;
}

void buffer_put(Buffer *buffer, int item)
{
    /* Counter before mutex, always: the inverse order deadlocks. */
    sem_wait(&buffer->empty);
    sem_wait(&buffer->mutex);

    buffer->slots[buffer->write_index] = item;
    buffer->write_index = (buffer->write_index + 1) % BUFFER_SIZE;

    sem_post(&buffer->mutex);
    sem_post(&buffer->full);
}

int buffer_take(Buffer *buffer)
{
    int item;

    sem_wait(&buffer->full);
    sem_wait(&buffer->mutex);

    item = buffer->slots[buffer->read_index];
    buffer->read_index = (buffer->read_index + 1) % BUFFER_SIZE;

    sem_post(&buffer->mutex);
    sem_post(&buffer->empty);

    return item;
}

void buffer_destroy(Buffer *buffer)
{
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);
    sem_destroy(&buffer->mutex);
}
