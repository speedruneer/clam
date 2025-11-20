#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <rypb.h>
#include <ryfs.h>

extern void ATA_READ(void *buf, uint32_t lba, uint32_t count);

/* ============================================================
   Basic helpers
   ============================================================ */

ryfs_node_t* ryfs_read_sector(rypb_t *rypb, int partition, uint32_t block_index) {
    if (!rypb) {printf("!rypb\n");return NULL;}
    if (partition < 0 || partition >= RYPB_MAX_PARTITIONS) {printf("partition < 0 || partition >= RYPB_MAX_PARTITIONS\n");return NULL;}

    uint32_t start_lba = rypb->partitions[partition].starting_lba + block_index - 1;

    ryfs_node_t *out = malloc(sizeof(ryfs_node_t));
    if (!out) {printf("!out\n");return NULL;}

    ATA_READ(out, start_lba, 1);
    return out;
}

void ryfs_free_node(ryfs_node_t *n) {
    free(n);
}

int ryfs_node_magic_is(ryfs_node_t *n, const char *magic4) {
    if (!n) return 0;
    return memcmp(n->data, magic4, 4) == 0;
}

/* ============================================================
   Node readers
   ============================================================ */

ryfs_root_t* ryfs_read_root(rypb_t *rypb, int partition, uint32_t block_index) {
    ryfs_node_t *raw = ryfs_read_sector(rypb, partition, block_index);
    if (!raw) return NULL;

    if (!ryfs_node_magic_is(raw, "ROOT")) {
        ryfs_free_node(raw);
        return NULL;
    }

    ryfs_root_t *r = malloc(sizeof(ryfs_root_t));
    memcpy(r, raw->data, sizeof(ryfs_root_t));
    ryfs_free_node(raw);
    return r;
}

ryfs_dirc_t* ryfs_read_dirc(rypb_t *rypb, int partition, uint32_t block_index) {
    ryfs_node_t *raw = ryfs_read_sector(rypb, partition, block_index);
    if (!raw) return NULL;

    if (!ryfs_node_magic_is(raw, "DIRC")) {
        ryfs_free_node(raw);
        return NULL;
    }

    ryfs_dirc_t *d = malloc(sizeof(ryfs_dirc_t));
    memcpy(d, raw->data, sizeof(ryfs_dirc_t));
    ryfs_free_node(raw);
    return d;
}

/* ============================================================
   Path resolver
   ============================================================ */

static char* dupstr(const char *s) {
    size_t l = strlen(s);
    char *r = malloc(l+1);
    memcpy(r, s, l+1);
    return r;
}

uint32_t ryfs_find_path(rypb_t *rypb, int partition, uint32_t root_block, const char *path) {
    if (!path || !*path) return 0;

    char *p = dupstr(path);

    while (*p == '/') p++;

    uint32_t current = root_block;

    char *seg = p;
    while (seg && *seg) {

        char *next = strchr(seg, '/');
        if (next) {
            *next = 0;
            next++;
            while (*next == '/') next++;
        }

        ryfs_node_t *node = ryfs_read_sector(rypb, partition, current);
        if (!node) { free(p); return 0; }

        uint32_t found = 0;

        if (ryfs_node_magic_is(node, "ROOT")) {
            uint32_t *arr = (uint32_t*)(node->data + 4);
            for (int i = 0; i < 127; i++) {
                if (!arr[i]) continue;

                ryfs_node_t *child = ryfs_read_sector(rypb, partition, arr[i]);
                if (!child) continue;

                if (ryfs_node_magic_is(child, "DIRC") || ryfs_node_magic_is(child, "FILE")) {
                    char name[65] = {0};
                    memcpy(name, child->data + 4, 64);

                    if (!strcmp(name, seg)) {
                        found = arr[i];
                        ryfs_free_node(child);
                        break;
                    }
                }

                ryfs_free_node(child);
            }
        }
        else if (ryfs_node_magic_is(node, "DIRC")) {
            uint32_t *arr = (uint32_t*)(node->data + 4 + 64);
            for (int i = 0; i < 111; i++) {
                if (!arr[i]) continue;

                ryfs_node_t *child = ryfs_read_sector(rypb, partition, arr[i]);
                if (!child) continue;

                if (ryfs_node_magic_is(child, "DIRC") || ryfs_node_magic_is(child, "FILE")) {
                    char name[65] = {0};
                    memcpy(name, child->data + 4, 64);

                    if (!strcmp(name, seg)) {
                        found = arr[i];
                        ryfs_free_node(child);
                        break;
                    }
                }

                ryfs_free_node(child);
            }
        }

        ryfs_free_node(node);

        if (!found) {
            free(p);
            return 0;
        }

        current = found;
        seg = next;
    }

    free(p);
    return current;
}

/* ============================================================
   File streaming
   ============================================================ */

ryfs_file_t_handle* ryfs_file_open(rypb_t *rypb, int partition, uint32_t file_block) {
    ryfs_node_t *n = ryfs_read_sector(rypb, partition, file_block);
    if (!n) return NULL;

    if (!ryfs_node_magic_is(n, "FILE")) {
        ryfs_free_node(n);
        printf("%x%x%x%x\n", n[0], n[1], n[2], n[3]);
        printf("node magic not FILE\n%s", (char*)n);
        return NULL;
    }

    ryfs_file_t_handle *h = malloc(sizeof(ryfs_file_t_handle));
    h->rypb = rypb;
    h->partition = partition;
    h->current_block = file_block;

    memcpy(&h->next_block, n->data + 4 + 64, 4);

    h->offset_in_block = 0;
    h->is_file_first_block = 1;
    h->node_buf = n;

    return h;
}

static int advance_block(ryfs_file_t_handle *h) {
    if (!h->next_block || h->next_block == h->current_block)
        return -1;

    ryfs_free_node(h->node_buf);

    ryfs_node_t *n = ryfs_read_sector(h->rypb, h->partition, h->next_block);
    if (!n) return -1;

    h->current_block = h->next_block;

    memcpy(&h->next_block, n->data + 4, 4);

    h->offset_in_block = 0;
    h->is_file_first_block = 0;
    h->node_buf = n;

    return 0;
}

ssize_t ryfs_file_read(ryfs_file_t_handle *h, void *buf, size_t bytes) {
    if (!h) printf("!h\n");
    if (!buf) printf("!buf\n");
    if (!h || !buf) {return -1;}

    uint8_t *out = buf;
    size_t read_total = 0;

    while (bytes > 0) {

        uint8_t *payload;
        size_t payload_size;

        if (h->is_file_first_block) {
            payload = h->node_buf->data + 72;
            payload_size = 512 - 72;
        } else {
            payload = h->node_buf->data + 8;
            payload_size = 512 - 8;
        }

        if (h->offset_in_block >= payload_size) {
            if (advance_block(h) < 0) break;
            continue;
        }

        size_t avail = payload_size - h->offset_in_block;
        size_t take = (avail < bytes) ? avail : bytes;

        memcpy(out + read_total, payload + h->offset_in_block, take);

        h->offset_in_block += take;
        read_total += take;
        bytes -= take;
    }

    return read_total;
}

void ryfs_file_close(ryfs_file_t_handle *h) {
    if (!h) return;
    if (h->node_buf) ryfs_free_node(h->node_buf);
    free(h);
}
