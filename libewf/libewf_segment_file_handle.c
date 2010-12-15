/*
 * Segment file handle functions
 *
 * Copyright (c) 2006-2010, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include <liberror.h>

#include "libewf_list_type.h"
#include "libewf_section_list.h"
#include "libewf_segment_file_handle.h"

/* Initialize the segment file handle
 * Returns 1 if successful or -1 on error
 */
int libewf_segment_file_handle_initialize(
     libewf_segment_file_handle_t **segment_file_handle,
     int file_io_pool_entry,
     liberror_error_t **error )
{
	static char *function = "libewf_segment_file_handle_initialize";

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file handle.",
		 function );

		return( -1 );
	}
	if( *segment_file_handle == NULL )
	{
		*segment_file_handle = memory_allocate_structure(
		                        libewf_segment_file_handle_t );

		if( *segment_file_handle == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create segment file handle.",
			 function );

			goto on_error;
		}
		if( memory_set(
		     *segment_file_handle,
		     0,
		     sizeof( libewf_segment_file_handle_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear segment file handle.",
			 function );

			goto on_error;
		}
		if( libewf_list_initialize(
		     &( ( *segment_file_handle )->section_list ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create section list.",
			 function );

			goto on_error;
		}
		( *segment_file_handle )->file_io_pool_entry = file_io_pool_entry;
	}
	return( 1 );

on_error:
	if( *segment_file_handle != NULL )
	{
		memory_free(
		 *segment_file_handle );

		*segment_file_handle = NULL;
	}
	return( -1 );
}

/* Frees the segment file handle including elements
 * Returns 1 if successful or -1 on error
 */
int libewf_segment_file_handle_free(
     intptr_t *segment_file_handle,
     liberror_error_t **error )
{
	static char *function = "libewf_segment_file_handle_free";
	int result            = 1;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file handle.",
		 function );

		return( -1 );
	}
	if( ( (libewf_segment_file_handle_t *) segment_file_handle )->section_list != NULL )
	{
		if( libewf_list_free(
		     &( ( (libewf_segment_file_handle_t *) segment_file_handle )->section_list ),
		     &libewf_section_list_values_free,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free section list.",
			 function );

			result = -1;
		}
	}
	memory_free(
	 segment_file_handle );

	return( result );
}

/* Clones the segment file handle
 * Returns 1 if successful or -1 on error
 */
int libewf_segment_file_handle_clone(
     intptr_t **destination_segment_file_handle,
     intptr_t *source_segment_file_handle,
     liberror_error_t **error )
{
	static char *function = "libewf_segment_file_handle_clone";

	if( destination_segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid destination segment file handle.",
		 function );

		return( -1 );
	}
	if( *destination_segment_file_handle != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid destination segment file handle value already set.",
		 function );

		return( -1 );
	}
	if( source_segment_file_handle == NULL )
	{
		*destination_segment_file_handle = NULL;

		return( 1 );
	}
	*destination_segment_file_handle = (intptr_t *) memory_allocate(
	                                                 sizeof( libewf_segment_file_handle_t ) );

	if( *destination_segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create destination segment file handle.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     *destination_segment_file_handle,
	     source_segment_file_handle,
	     sizeof( libewf_segment_file_handle_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy source to destination segment file handle.",
		 function );

		goto on_error;
	}
	( (libewf_segment_file_handle_t *) *destination_segment_file_handle )->section_list = NULL;

	if( libewf_list_clone(
	     &( ( (libewf_segment_file_handle_t *) *destination_segment_file_handle )->section_list ),
	     ( (libewf_segment_file_handle_t *) source_segment_file_handle )->section_list,
	     &libewf_section_list_values_free,
	     &libewf_section_list_values_clone,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create destination section list.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( *destination_segment_file_handle != NULL )
	{
		memory_free(
		 *destination_segment_file_handle );

		*destination_segment_file_handle = NULL;
	}
	return( -1 );
}

