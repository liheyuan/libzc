/*
 *  zc - zip crack library
 *  Copyright (C) 2009  Marc Ferland
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

#ifndef _LIBZC_H_
#define _LIBZC_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * zc_ctx
 *
 * library user context - reads the config and system
 * environment, user variables, allows custom logging
 */
struct zc_ctx;
struct zc_ctx *zc_ref(struct zc_ctx *ctx);
struct zc_ctx *zc_unref(struct zc_ctx *ctx);
int zc_new(struct zc_ctx **inctx);
void zc_set_log_fn(struct zc_ctx *ctx,
                  void (*log_fn)(struct zc_ctx *ctx,
                                 int priority, const char *file, int line, const char *fn,
                                 const char *format, va_list args));
int zc_get_log_priority(struct zc_ctx *ctx);
void zc_set_log_priority(struct zc_ctx *ctx, int priority);

/*
 * zc_file
 *
 * contains information about the zip file
 */
struct zc_file;
struct zc_file *zc_file_ref(struct zc_file *file);
struct zc_file *zc_file_unref(struct zc_file *file);
/* struct zc_ctx *zc_file_get_ctx(struct zc_file *file); */
int zc_file_new_from_filename(struct zc_ctx *ctx, const char *filename, struct zc_file **file);
const char *zc_file_get_filename(const struct zc_file *file);
int zc_file_open(struct zc_file *file);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
