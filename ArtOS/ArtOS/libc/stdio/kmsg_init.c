#include <stdio.h>


static ssize_t kmsg_read(fd_t* file, uint8_t* buffer, size_t size)
{
    size_t start, i = 0;
    
    if (BUILTIN_EXPECT(!buffer, 0))
        return -EINVAL;
    if (BUILTIN_EXPECT(!size, 0))
        return 0;
    
    if (kmessages[(atomic_int32_read(&kmsg_counter) + 1) % KMSG_SIZE] == 0)
        start = 0;
    else
        start = (atomic_int32_read(&kmsg_counter) + 1) % KMSG_SIZE;
    
    if (((start + file->offset) % KMSG_SIZE) == atomic_int32_read(&kmsg_counter))
        return 0;
    if (file->offset >= KMSG_SIZE)
        return 0;
    
    for(i=0; i<size; i++, file->offset++) {
        buffer[i] = kmessages[(start + file->offset) % KMSG_SIZE];
        if (((start + file->offset) % KMSG_SIZE) == atomic_int32_read(&kmsg_counter))
            return i;
    }
    
    return size;
}

static int kmsg_open(fd_t* file, const char *name)
{
    return 0;
}

static int kmsg_close(fd_t* file)
{
    return 0;
}

/* Init Functions */
int kmsg_init(vfs_node_t * node, const char *name)
{
    uint32_t        i, j;
    vfs_node_t*     new_node;
    dir_block_t*    blockdir;
    dirent_t*       dirent;
    block_list_t*   blist;
    
    if (BUILTIN_EXPECT(!node || !name, 0))
        return -EINVAL;
    
    if (BUILTIN_EXPECT(node->type != FS_DIRECTORY, 0))
        return -EINVAL;
    
    if (finddir_fs(node, name))
        return -EINVAL;
    
    new_node = kmalloc(sizeof(vfs_node_t));
    if (BUILTIN_EXPECT(!new_node, 0))
        return -ENOMEM;
    
    memset(new_node, 0x00, sizeof(vfs_node_t));
    new_node->type = FS_CHARDEVICE;
    new_node->open = &kmsg_open;
    new_node->close = &kmsg_close;
    new_node->read = &kmsg_read;
    new_node->write = NULL;
    lock_init(&new_node->lock);
    
    blist = &node->block_list;
    do {
        for (i = 0; i < MAX_DATABLOCKS; i++) {
            if (blist->data[i]) {
                blockdir = (dir_block_t *) blist->data[i];
                for (j = 0; j < MAX_DIRENTRIES; j++) {
                    dirent = &blockdir->entries[j];
                    if (!dirent->vfs_node) {
                        dirent->vfs_node = new_node;
                        strncpy(dirent->name, name, MAX_FILENAME);
                        return 0;
                    }
                }
            }
        }
        
        if (!blist->next) {
            blist->next = (block_list_t *) kmalloc(sizeof(block_list_t));
            if (blist->next)
                memset(blist->next, 0x00, sizeof(block_list_t));
        }
    } while (blist);
    
    kfree(new_node);
    
    return -ENOMEM;
}
