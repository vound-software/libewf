/*
 * libewf file io pool
 *
 * Copyright (c) 2006-2008, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the creator, related organisations, nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER, COMPANY AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "libewf_includes.h"

#include <libewf/libewf_definitions.h>

#include "libewf_common.h"
#include "libewf_file_io_pool.h"
#include "libewf_notify.h"
#include "libewf_string.h"

/* Allocates memory for a file io pool struct
 * Returns a pointer to the new instance, NULL on error
 */
LIBEWF_FILE_IO_POOL *libewf_file_io_pool_alloc( size_t amount )
{
	LIBEWF_FILE_IO_POOL *file_io_pool = NULL;
	static char *function             = "libewf_file_io_pool_alloc";
	size_t iterator                   = 0;

	file_io_pool = (LIBEWF_FILE_IO_POOL *) libewf_common_alloc( LIBEWF_FILE_IO_POOL_SIZE );

	if( file_io_pool == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate file io pool.\n",
		 function );

		return( NULL );
	}
	file_io_pool->handle = (LIBEWF_FILE_IO_HANDLE *) libewf_common_alloc( amount * LIBEWF_FILE_IO_HANDLE_SIZE );

	if( file_io_pool->handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate file io handles.\n",
		 function );

		libewf_common_free( file_io_pool );

		return( NULL );
	}
	for( iterator = 0; iterator < amount; iterator++ )
	{
		file_io_pool->handle[ iterator ].filename        = NULL;
		file_io_pool->handle[ iterator ].file_descriptor = -1;
		file_io_pool->handle[ iterator ].file_offset     = 0;
		file_io_pool->handle[ iterator ].flags           = 0;
	}
	file_io_pool->amount     = amount;
	file_io_pool->open_files = 0;

	return( file_io_pool );
}

/* Reallocates memory for the file io pool entries
 * Returns 1 if successful, or -1 on error
 */
int libewf_file_io_pool_realloc( LIBEWF_FILE_IO_POOL *file_io_pool, size_t amount )
{
	void *reallocation    = NULL;
	static char *function = "libewf_file_io_pool_realloc";
	size_t iterator       = 0;

	if( file_io_pool == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file io pool.\n",
		 function );

		return( -1 );
	}
	if( file_io_pool->amount >= amount )
	{
		LIBEWF_WARNING_PRINT( "%s: new amount must be greater than previous amount.\n",
		 function );

		return( -1 );
	}
	reallocation = libewf_common_realloc(
	                file_io_pool->handle,
	                ( amount * LIBEWF_FILE_IO_HANDLE_SIZE ) );

	if( reallocation == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to reallocate dynamic file io handles array.\n",
		 function );

		return( -1 );
	}
	file_io_pool->handle = (LIBEWF_FILE_IO_HANDLE *) reallocation;

	for( iterator = file_io_pool->amount; iterator < amount; iterator++ )
	{
		file_io_pool->handle[ iterator ].filename        = NULL;
		file_io_pool->handle[ iterator ].file_descriptor = -1;
		file_io_pool->handle[ iterator ].file_offset     = 0;
		file_io_pool->handle[ iterator ].flags           = 0;
	}
	file_io_pool->amount = amount;

	return( 1 );
}

/* Frees memory of a file io pool
 */
void libewf_file_io_pool_free( LIBEWF_FILE_IO_POOL *file_io_pool )
{
	static char *function = "libewf_file_io_pool_free";
	size_t iterator       = 0;

	if( file_io_pool == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file io pool.\n",
		 function );

		return;
	}
	for( iterator = 0; iterator < file_io_pool->amount; iterator++ )
	{
		if( file_io_pool->handle[ iterator ].filename != NULL )
		{
			libewf_common_free( file_io_pool->handle[ iterator ].filename );
		}
	}
	libewf_common_free( file_io_pool->handle );
	libewf_common_free( file_io_pool );
}

int libewf_file_io_pool_open( LIBEWF_FILE_IO_POOL *file_io_pool, LIBEWF_FILENAME *filename, int flags );

ssize_t libewf_file_io_pool_read( LIBEWF_FILE_IO_POOL *pool, size_t entry, uint8_t *buffer, size_t size );
ssize_t libewf_file_io_pool_write( LIBEWF_FILE_IO_POOL *pool, size_t entry, uint8_t *buffer, size_t size );
off64_t libewf_file_io_pool_seek( LIBEWF_FILE_IO_POOL *pool, size_t entry, off64_t offset, int whence );
int libewf_file_io_pool_close( LIBEWF_FILE_IO_POOL *pool, size_t entry );

