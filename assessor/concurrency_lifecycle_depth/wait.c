#include "wait.h"
#ifndef CW_MUTANT
#define CW_MUTANT 0
#endif

static int due(uint32_t now, cmx_deadline deadline)
{
    return !deadline.infinite &&
           (uint32_t)(now - deadline.at) < UINT32_C(0x80000000);
}

void cw_init(struct cw_world *world, cmx_kernel *kernel)
{
    *world = (struct cw_world){0};
    world->kernel = kernel;
    for (unsigned i = 0; i < CMX_TASKS; ++i)
        world->nodes[i].result = CMX_INVALID;
}

static int object_valid(const struct cw_world *world, unsigned index)
{
    return world && world->kernel && index < 2 && world->objects[index].live;
}

static int matches(struct cw_token token)
{
    if (!token.owner || !token.owner->kernel || token.object >= 2 ||
        !token.owner->objects[token.object].generation ||
        token.task >= CMX_TASKS)
        return 0;
    const struct cw_node *node = &token.owner->nodes[token.task];
    return token.operation &&
           (CW_MUTANT == 1 || token.operation == node->sequence) &&
           token.task_generation == node->task_generation &&
           token.task_generation ==
               token.owner->kernel->task[token.task].generation &&
           token.object == node->object &&
           token.object_generation == node->object_generation &&
           token.object_generation ==
               token.owner->objects[token.object].generation;
}

static void synchronize(struct cw_world *world, unsigned reason)
{
    for (unsigned i = 0; i < CMX_TASKS; ++i)
    {
        struct cw_node *node = &world->nodes[i];
        const cmx_task *task = &world->kernel->task[i];
        if (!node->linked || task->state == CMX_WAITING)
            continue;
        node->linked = 0;
        node->result = task->result;
        node->cause = task->result == CMX_OK
                          ? CW_READY
                          : (task->result == CMX_TIMEOUT ? CW_TIMEOUT : reason);
        node->terminals += CW_MUTANT == 2 ? 2 : 1;
        if (task->result == CMX_OK)
            ++world->objects[node->object].acquired;
    }
}

int cw_object_init(struct cw_world *world, unsigned index, unsigned kind,
                   unsigned initial, unsigned maximum)
{
    if (!world || !world->kernel || index >= 2 || kind > CW_MUTEX ||
        (kind == CW_SEMAPHORE && (!maximum || initial > maximum)))
        return CMX_INVALID;
    struct cw_object *object = &world->objects[index];
    if (object->live)
        return CMX_INVALID;
    if (object->generation == UINT32_MAX)
        return CMX_FULL;
    if (kind == CW_MUTEX)
    {
        int result = cmx_mutex_init(world->kernel, &object->mutex, 0, 0);
        if (result != CMX_OK)
            return result;
    }
    else
        cmx_sem_init(&object->semaphore, initial, maximum);
    ++object->generation;
    object->kind = kind;
    object->live = 1;
    object->posts = 0;
    object->acquired = 0;
    return CMX_OK;
}

int cw_begin(struct cw_world *world, unsigned index, unsigned task,
             cmx_deadline deadline, unsigned strict, struct cw_token *out)
{
    if (!object_valid(world, index) || !out || task >= CMX_TASKS || strict > 1)
        return CMX_INVALID;
    struct cw_node *node = &world->nodes[task];
    cmx_task *entry = &world->kernel->task[task];
    if (node->linked ||
        (entry->state != CMX_READY && entry->state != CMX_RUNNING))
        return CMX_INVALID;
    if (node->sequence == UINT32_MAX || node->terminals == UINT64_MAX)
        return CMX_FULL;
    if (strict && due(world->kernel->now, deadline) && CW_MUTANT != 4)
        return CMX_TIMEOUT;
    struct cw_object *object = &world->objects[index];
    /* Reserve counter room before the underlying operation can acquire. */
    if (object->acquired == UINT64_MAX)
        return CMX_FULL;
    unsigned saved = object->semaphore.count;
    if (CW_MUTANT == 3 && object->kind == CW_SEMAPHORE)
        object->semaphore.count = 0;
    int result =
        object->kind == CW_SEMAPHORE
            ? cmx_sem_take(world->kernel, &object->semaphore, task, deadline)
            : cmx_mutex_lock(world->kernel, &object->mutex, task, deadline);
    if (CW_MUTANT == 3 && object->kind == CW_SEMAPHORE)
        object->semaphore.count = saved;
    if (result != CMX_OK && result != CMX_BLOCKED)
        return result;
    ++node->sequence;
    node->object = index;
    node->object_generation = object->generation;
    node->task_generation = entry->generation;
    node->linked = result == CMX_BLOCKED;
    node->result = result;
    node->cause = result == CMX_OK ? CW_READY : CW_NONE;
    if (result == CMX_OK)
    {
        ++node->terminals;
        ++object->acquired;
    }
    *out = (struct cw_token){
        world,         task, index, entry->generation, object->generation,
        node->sequence};
    return result;
}

int cw_result(struct cw_token token, int *result, unsigned *cause)
{
    if (!result || !cause)
        return CMX_INVALID;
    if (!matches(token))
        return CW_STALE;
    *result = token.owner->nodes[token.task].result;
    *cause = token.owner->nodes[token.task].cause;
    return CMX_OK;
}

int cw_post(struct cw_world *world, unsigned index)
{
    if (!object_valid(world, index) ||
        world->objects[index].kind != CW_SEMAPHORE)
        return CMX_INVALID;
    struct cw_object *object = &world->objects[index];
    if (object->posts == UINT64_MAX || object->acquired == UINT64_MAX)
        return CMX_FULL;
    int result = cmx_sem_give(world->kernel, &object->semaphore);
    if (result == CMX_OK)
        ++object->posts;
    synchronize(world, CW_READY);
    return result;
}

int cw_ready_callback(struct cw_token token)
{
    if (!matches(token) || !token.owner->nodes[token.task].linked)
        return CW_STALE;
    return cw_post(token.owner, token.object);
}

int cw_timeout(struct cw_token token, uint32_t now)
{
    if (!matches(token) || !token.owner->nodes[token.task].linked)
        return CW_STALE;
    cmx_kernel *kernel = token.owner->kernel;
    if ((uint32_t)(now - kernel->now) >= UINT32_C(0x80000000) ||
        !due(now, kernel->task[token.task].deadline))
        return CMX_INVALID;
    cmx_tick(kernel, now);
    synchronize(token.owner, CW_TIMEOUT);
    return CMX_OK;
}

int cw_cancel(struct cw_token token)
{
    if (!matches(token))
        return CW_STALE;
    struct cw_object *object = &token.owner->objects[token.object];
    if (!token.owner->nodes[token.task].linked)
    {
        if (CW_MUTANT == 5 && object->kind == CW_MUTEX)
            return cmx_mutex_unlock(token.owner->kernel, &object->mutex,
                                    token.task);
        return CW_STALE;
    }
    int owner = object->kind == CW_MUTEX ? object->mutex.owner : CMX_NONE;
    unsigned priority =
        owner >= 0 ? token.owner->kernel->task[owner].priority : 0;
    int result = cmx_cancel(token.owner->kernel, token.task);
    if (CW_MUTANT == 6 && owner >= 0)
        token.owner->kernel->task[owner].priority = priority;
    synchronize(token.owner, CW_CANCEL);
    return result;
}

int cw_unlock(struct cw_world *world, unsigned index, unsigned task)
{
    if (!object_valid(world, index) || world->objects[index].kind != CW_MUTEX ||
        task >= CMX_TASKS)
        return CMX_INVALID;
    cmx_mutex *mutex = &world->objects[index].mutex;
    if (mutex->owner != (int)task)
        return CMX_NOT_OWNER;
    if (world->objects[index].acquired == UINT64_MAX)
    {
        if (CW_MUTANT == 8)
            return CMX_FULL;
        for (unsigned t = 0; t < CMX_TASKS; ++t)
        {
            const cmx_task *entry = &world->kernel->task[t];
            if (entry->state == CMX_WAITING && entry->wait == CMX_W_MUTEX &&
                entry->object == mutex)
                return CMX_FULL;
        }
    }
    int result =
        cmx_mutex_unlock(world->kernel, &world->objects[index].mutex, task);
    synchronize(world, CW_READY);
    return result;
}

int cw_close(struct cw_world *world, unsigned index)
{
    if (!object_valid(world, index))
        return CMX_INVALID;
    struct cw_object *object = &world->objects[index];
    int result = object->kind == CW_SEMAPHORE
                     ? cmx_sem_close(world->kernel, &object->semaphore)
                     : cmx_mutex_close(world->kernel, &object->mutex);
    synchronize(world, CW_CLOSE);
    if (CW_MUTANT == 7 && result == CMX_OK)
        for (unsigned i = 0; i < CMX_TASKS; ++i)
            if (world->nodes[i].sequence && world->nodes[i].object == index)
            {
                world->nodes[i].result = CMX_CLOSED;
                world->nodes[i].cause = CW_CLOSE;
            }
    /* Tokens remain queryable until explicit reinitialization. */
    if (result == CMX_OK)
        object->live = 0;
    return result;
}

int cw_exit_reap(struct cw_world *world, unsigned task)
{
    if (!world || !world->kernel || task >= CMX_TASKS ||
        world->nodes[task].linked)
        return CMX_INVALID;
    if (world->kernel->current == (int)task)
        return CMX_INVALID;
    if (world->kernel->task[task].generation == UINT32_MAX)
        return CMX_FULL;
    int result = cmx_exit(world->kernel, task, 0);
    if (result == CMX_OK)
        return cmx_reap(world->kernel, task);
    return result;
}
