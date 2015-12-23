/*
 *  zc - zip crack library
 *  Copyright (C) 2013  Marc Ferland
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "zip.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define ZIP_SIG               0x04034b50
#define ZIP_DATA_DESC_SIG     0x08074b50
#define ZIP_STATIC_HEADER_LEN 30
#define GP_BIT_HAS_DATA_DESC  (1 << 3)
#define GP_BIT_ENCRYPTION     0x1

struct zip_header {
    uint16_t version_needed;
    uint16_t gen_bit_flag;
    uint16_t comp_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t crc32;
    uint32_t comp_size;
    uint32_t uncomp_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
    char *filename;
};

static int check_header_signature(FILE *fd)
{
    int sig;
    if (fread(&sig, 4, 1, fd) != 1)
        return -1;
    return sig == ZIP_SIG ? 0 : -1;
}

static uint16_t get_le16_at(const uint8_t *b, size_t i)
{
    return b[i + 1] << 8 | b[i];
}

static uint32_t get_le32_at(const uint8_t *b, size_t i)
{
    return b[i + 3] << 24 | b[i + 2] << 16 | b[i + 1] << 8 | b[i];
}

static int zip_header_read_static_part(FILE *fd, struct zip_header *h)
{
    uint8_t buf[ZIP_STATIC_HEADER_LEN - 4];

    if (fread(buf, ZIP_STATIC_HEADER_LEN - 4, 1, fd) != 1)
        return -1;

    h->version_needed = get_le16_at(buf, 0);
    h->gen_bit_flag = get_le16_at(buf, 2);
    h->comp_method = get_le16_at(buf, 4);
    h->last_mod_time = get_le16_at(buf, 6);
    h->last_mod_date = get_le16_at(buf, 8);
    h->crc32 = get_le32_at(buf, 10);
    h->comp_size = get_le32_at(buf, 14);
    h->uncomp_size = get_le32_at(buf, 18);
    h->filename_length = get_le32_at(buf, 22);
    h->extra_field_length = get_le32_at(buf, 24);

    return 0;
}

static int zip_header_read_variable_part(FILE *fd, struct zip_header *header)
{
    const size_t filename_size = header->filename_length + 1;

    header->filename = realloc(header->filename, filename_size);
    if (!header->filename)
        return -1;
    memset(header->filename, 0, filename_size);

    if (fread(header->filename, header->filename_length, 1, fd) != 1) {
        free(header->filename);
        return -1;
    }

    // Skip the extra field since we do not use it
    fseek(fd, header->extra_field_length, SEEK_CUR);

    return 0;
}

static uint32_t zip_header_get_data_size(const struct zip_header *header)
{
    if (zip_header_has_encryption_bit(header))
        return header->comp_size - ZIP_ENCRYPTION_HEADER_LENGTH;
    return header->comp_size;
}

static bool zip_header_has_data_descriptor_bit(const struct zip_header *header)
{
    return (header->gen_bit_flag & GP_BIT_HAS_DATA_DESC);
}

static int zip_skip_data(FILE *fd, const struct zip_header *header)
{
    int err;

    if ((err = fseek(fd, zip_header_get_data_size(header), SEEK_CUR)) != 0)
        return err;
    return 0;
}

static int zip_skip_data_descriptor(FILE *fd)
{
    /*
      signature                       4 bytes (optional)
      crc-32                          4 bytes
      compressed size                 4 bytes
      uncompressed size               4 bytes
     */
    int sig;

    if (fread(&sig, 4, 1, fd) != 1)
        return -1;
    return fseek(fd, sig == ZIP_DATA_DESC_SIG ? 12 : 8, SEEK_CUR);
}

int zip_header_new(struct zip_header **header)
{
    struct zip_header *newheader;

    newheader = calloc(1, sizeof(struct zip_header));
    if (!newheader)
        return -1;

    *header = newheader;
    return 0;
}

void zip_header_free(struct zip_header *header)
{
    if (header->filename)
        free(header->filename);
    free(header);
}

int zip_header_read(FILE *fd, struct zip_header *header)
{
    int err;

    if ((err = check_header_signature(fd)) != 0)
        return err;

    if ((err = zip_header_read_static_part(fd, header)) != 0)
        return err;

    if ((err = zip_header_read_variable_part(fd, header)) != 0)
        return err;

    return 0;
}

bool zip_header_has_encryption_bit(const struct zip_header *header)
{
    return ((header->gen_bit_flag & GP_BIT_ENCRYPTION) == GP_BIT_ENCRYPTION);
}

uint32_t zip_header_comp_size(const struct zip_header *header)
{
    return zip_header_get_data_size(header);
}

uint8_t zip_header_encryption_magic(const struct zip_header *header)
{
    /*
      Nothing about this in the official APPNOTE.txt even though the
      majority of unzippers check for both fields. This code was
      inspired from funzip.
    */
    if (zip_header_has_data_descriptor_bit(header))
        return (header->last_mod_time >> 8);
    return (header->crc32 >> 24);
}

int zip_encryption_header_read(FILE *fd, uint8_t *enc_header)
{
    if (fread(enc_header, ZIP_ENCRYPTION_HEADER_LENGTH, 1, fd) != 1)
        return -1;
    return 0;
}

int zip_encryption_header_skip(FILE *fd)
{
    return fseek(fd, ZIP_ENCRYPTION_HEADER_LENGTH, SEEK_CUR);
}

int zip_skip_to_next_header(FILE *fd, const struct zip_header *header)
{
    if (zip_skip_data(fd, header))
        return -1;

    if (zip_header_has_data_descriptor_bit(header)) {
        if (zip_skip_data_descriptor(fd))
            return -1;
    }
    return 0;
}

const char *zip_header_filename(const struct zip_header *header)
{
    return header->filename;
}

size_t zip_header_filename_len(const struct zip_header *header)
{
    return header->filename_length;
}
