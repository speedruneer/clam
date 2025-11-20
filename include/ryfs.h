#pragma once
#ifndef RYFS_H
#define RYFS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <rypb.h>

typedef int ssize_t;

/* ============================================================
   On-disk node structures (exact 512-byte blocks)
   ============================================================ */

typedef struct __attribute__((packed)) ryfs_root_node {
    char magic[4];             // "ROOT"
    uint32_t children[127];    // 127 * 4 = 508 bytes
} ryfs_root_t;

typedef struct __attribute__((packed)) ryfs_dirc_node {
    char magic[4];             // "DIRC"
    char filename[64];         // NTS string
    uint32_t children[111];    // aligns to 512
} ryfs_dirc_t;

typedef struct __attribute__((packed)) ryfs_file_node {
    char magic[4];             // "FILE"
    char filename[64];
    uint32_t next_block_id;
    uint8_t data[512 - (4 + 64 + 4)];
} ryfs_file_t;

typedef struct __attribute__((packed)) ryfs_file_data_node {
    char magic[4];             // "DATA"
    uint32_t next_block_id;
    uint8_t data[512 - (4 + 4)];
} ryfs_data_t;

typedef struct __attribute__((packed)) ryfs_node {
    uint8_t data[512];
} ryfs_node_t;

/* ============================================================
   File streaming handle
   ============================================================ */

typedef struct ryfs_file_handle {
    rypb_t *rypb;
    int partition;

    uint32_t current_block;
    uint32_t next_block;
    size_t offset_in_block;

    int is_file_first_block;

    ryfs_node_t *node_buf;
} ryfs_file_t_handle;

/* ============================================================
   API
   ============================================================ */

ryfs_node_t* ryfs_read_sector(rypb_t *rypb, int partition, uint32_t block_index);
void ryfs_free_node(ryfs_node_t *n);

int ryfs_node_magic_is(ryfs_node_t *n, const char *magic4);

/* root/directory */
ryfs_root_t* ryfs_read_root(rypb_t *rypb, int partition, uint32_t block_index);
ryfs_dirc_t* ryfs_read_dirc(rypb_t *rypb, int partition, uint32_t block_index);

uint32_t ryfs_find_path(rypb_t *rypb, int partition, uint32_t root_block_index, const char *path);

/* file streaming */
ryfs_file_t_handle* ryfs_file_open(rypb_t *rypb, int partition, uint32_t file_block_index);
ssize_t ryfs_file_read(ryfs_file_t_handle *h, void *buf, size_t bytes);
void ryfs_file_close(ryfs_file_t_handle *h);

#ifdef __cplusplus
}
#endif

#endif
