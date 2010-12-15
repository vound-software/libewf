/* 
 * Info handle
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
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>

#if defined( HAVE_SYS_UTSNAME_H )
#include <sys/utsname.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include <libsystem.h>

#include "byte_size_string.h"
#include "digest_hash.h"
#include "ewfinput.h"
#include "guid.h"
#include "info_handle.h"

#define INFO_HANDLE_VALUE_SIZE			512
#define INFO_HANDLE_VALUE_IDENTIFIER_SIZE	64
#define INFO_HANDLE_NOTIFY_STREAM		stdout

#if !defined( USE_LIBEWF_GET_HASH_VALUE_MD5 ) && !defined( USE_LIBEWF_GET_MD5_HASH )
#define USE_LIBEWF_GET_HASH_VALUE_MD5
#endif

/* Initializes the info handle
 * Returns 1 if successful or -1 on error
 */
int info_handle_initialize(
     info_handle_t **info_handle,
     liberror_error_t **error )
{
	static char *function = "info_handle_initialize";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( *info_handle == NULL )
	{
		*info_handle = memory_allocate_structure(
		                info_handle_t );

		if( *info_handle == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create info handle.",
			 function );

			goto on_error;
		}
		if( memory_set(
		     *info_handle,
		     0,
		     sizeof( info_handle_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear info handle.",
			 function );

			goto on_error;
		}
		if( libewf_handle_initialize(
		     &( ( *info_handle )->input_handle ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to initialize input handle.",
			 function );

			goto on_error;
		}
		( *info_handle )->output_format   = INFO_HANDLE_OUTPUT_FORMAT_TEXT;
		( *info_handle )->date_format     = LIBEWF_DATE_FORMAT_CTIME;
		( *info_handle )->header_codepage = LIBEWF_CODEPAGE_ASCII;
		( *info_handle )->notify_stream   = INFO_HANDLE_NOTIFY_STREAM;
	}
	return( 1 );

on_error:
	if( *info_handle != NULL )
	{
		memory_free(
		 *info_handle );

		*info_handle = NULL;
	}
	return( -1 );
}

/* Frees the info handle and its elements
 * Returns 1 if successful or -1 on error
 */
int info_handle_free(
     info_handle_t **info_handle,
     liberror_error_t **error )
{
	static char *function = "info_handle_free";
	int result            = 1;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( *info_handle != NULL )
	{
		if( ( ( *info_handle )->input_handle != NULL )
		 && ( libewf_handle_free(
		       &( ( *info_handle )->input_handle ),
		       error ) != 1 ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free input handle.",
			 function );

			result = -1;
		}
		memory_free(
		 *info_handle );

		*info_handle = NULL;
	}
	return( result );
}

/* Signals the info handle to abort
 * Returns 1 if successful or -1 on error
 */
int info_handle_signal_abort(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	static char *function = "info_handle_signal_abort";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle != NULL )
	{
		if( libewf_handle_signal_abort(
		     info_handle->input_handle,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to signal input handle to abort.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Opens the input of the info handle
 * Returns 1 if successful or -1 on error
 */
int info_handle_open_input(
     info_handle_t *info_handle,
     libcstring_system_character_t * const * filenames,
     int number_of_filenames,
     liberror_error_t **error )
{
	libcstring_system_character_t **libewf_filenames = NULL;
	static char *function                            = "info_handle_open_input";
	size_t first_filename_length                     = 0;
	int filename_index                               = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( filenames == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filenames.",
		 function );

		return( -1 );
	}
	if( number_of_filenames <= 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid number of filenames.",
		 function );

		return( -1 );
	}
	if( number_of_filenames == 1 )
	{
		first_filename_length = libcstring_system_string_length(
		                         filenames[ 0 ] );

#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		if( libewf_glob_wide(
		     filenames[ 0 ],
		     first_filename_length,
		     LIBEWF_FORMAT_UNKNOWN,
		     &libewf_filenames,
		     &number_of_filenames,
		     error ) != 1 )
#else
		if( libewf_glob(
		     filenames[ 0 ],
		     first_filename_length,
		     LIBEWF_FORMAT_UNKNOWN,
		     &libewf_filenames,
		     &number_of_filenames,
		     error ) != 1 )
#endif
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to resolve filename(s).",
			 function );

			goto on_error;
		}
		filenames = (libcstring_system_character_t * const *) libewf_filenames;
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	if( libewf_handle_open_wide(
	     info_handle->input_handle,
	     filenames,
	     number_of_filenames,
	     LIBEWF_OPEN_READ,
	     error ) != 1 )
#else
	if( libewf_handle_open(
	     info_handle->input_handle,
	     filenames,
	     number_of_filenames,
	     LIBEWF_OPEN_READ,
	     error ) != 1 )
#endif
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open file(s).",
		 function );

		goto on_error;
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t<image_filenames>\n" );

		for( filename_index = 0;
		     filename_index < number_of_filenames;
		     filename_index++ )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t\t\t<image_filename>%" PRIs_LIBCSTRING_SYSTEM "</image_filename>\n",
			 filenames[ filename_index ] );
		}
		fprintf(
		 info_handle->notify_stream,
		 "\t\t</image_filenames>\n" );
	}
	if( info_handle->header_codepage != LIBEWF_CODEPAGE_ASCII )
	{
		if( libewf_handle_set_header_codepage(
		     info_handle->input_handle,
		     info_handle->header_codepage,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set header codepage.",
			 function );

			goto on_error;
		}
	}
	if( libewf_filenames != NULL )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		if( libewf_glob_wide_free(
		     libewf_filenames,
		     number_of_filenames,
		     error ) != 1 )
#else
		if( libewf_glob_free(
		     libewf_filenames,
		     number_of_filenames,
		     error ) != 1 )
#endif
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free globbed filenames.",
			 function );

			return( -1 );
		}
		libewf_filenames = NULL;
	}
	return( 1 );

on_error:
	if( libewf_filenames != NULL )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		libewf_glob_wide_free(
		 libewf_filenames,
		 number_of_filenames,
		 NULL );
#else
		libewf_glob_free(
		 libewf_filenames,
		 number_of_filenames,
		 NULL );
#endif
	}
	return( -1 );
}

/* Closes the info handle
 * Returns the 0 if succesful or -1 on error
 */
int info_handle_close(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	static char *function = "info_handle_close";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( libewf_handle_close(
	     info_handle->input_handle,
	     error ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_CLOSE_FAILED,
		 "%s: unable to close input handle.",
		 function );

		return( -1 );
	}
	return( 0 );
}

/* Sets the output format
 * Returns 1 if successful or -1 on error
 */
int info_handle_set_output_format(
     info_handle_t *info_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error )
{
	static char *function = "info_handle_set_output_format";
	size_t string_length  = 0;
	int result            = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	string_length = libcstring_system_string_length(
	                 string );

	if( string_length == 4 )
	{
		if( libcstring_system_string_compare(
		     string,
		     _LIBCSTRING_SYSTEM_STRING( "text" ),
		     4 ) == 0 )
		{
			info_handle->output_format = INFO_HANDLE_OUTPUT_FORMAT_TEXT;
			result                     = 1;
		}
	}
	else if( string_length == 5 )
	{
		if( libcstring_system_string_compare(
		     string,
		     _LIBCSTRING_SYSTEM_STRING( "dfxml" ),
		     5 ) == 0 )
		{
			info_handle->output_format = INFO_HANDLE_OUTPUT_FORMAT_DFXML;
			info_handle->date_format   = LIBEWF_DATE_FORMAT_ISO8601;
			result                     = 1;
		}
	}
	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine output format.",
		 function );

		return( -1 );
	}
	return( result );
}

/* Sets the date format
 * Returns 1 if successful, 0 if unsupported values or -1 on error
 */
int info_handle_set_date_format(
     info_handle_t *info_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error )
{
	static char *function = "info_handle_set_date_format";
	size_t string_length  = 0;
	int result            = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	string_length = libcstring_system_string_length(
	                 string );

	if( string_length == 2 )
	{
		if( libcstring_system_string_compare(
		     string,
		     _LIBCSTRING_SYSTEM_STRING( "dm" ),
		     2 ) == 0 )
		{
			info_handle->date_format = LIBEWF_DATE_FORMAT_DAYMONTH;
			result                   = 1;
		}
		else if( libcstring_system_string_compare(
			  string,
			  _LIBCSTRING_SYSTEM_STRING( "md" ),
			  3 ) == 0 )
		{
			info_handle->date_format = LIBEWF_DATE_FORMAT_MONTHDAY;
			result                   = 1;
		}
	}
	else if( string_length == 5 )
	{
		if( libcstring_system_string_compare(
		     string,
		     _LIBCSTRING_SYSTEM_STRING( "ctime" ),
		     5 ) == 0 )
		{
			info_handle->date_format = LIBEWF_DATE_FORMAT_CTIME;
			result                   = 1;
		}
	}
	else if( string_length == 7 )
	{
		if( libcstring_system_string_compare(
		     string,
		     _LIBCSTRING_SYSTEM_STRING( "iso8601" ),
		     7 ) == 0 )
		{
			info_handle->date_format = LIBEWF_DATE_FORMAT_ISO8601;
			result                   = 1;
		}
	}
	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine date format.",
		 function );

		return( -1 );
	}
	return( result );
}

/* Sets the header codepage
 * Returns 1 if successful or -1 on error
 */
int info_handle_set_header_codepage(
     info_handle_t *info_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error )
{
	static char *function = "info_handle_set_header_codepage";
	int result            = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	result = ewfinput_determine_header_codepage(
	          string,
	          &( info_handle->header_codepage ),
	          error );

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine header codepage.",
		 function );

		return( -1 );
	}
	else if( result != 0 )
	{
		if( info_handle->input_handle != NULL )
		{
			if( libewf_handle_set_header_codepage(
			     info_handle->input_handle,
			     info_handle->header_codepage,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set header codepage.",
				 function );

				return( -1 );
			}
		}
	}
	return( result );
}

/* Prints a section header to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_section_header_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     const char *description,
     liberror_error_t **error )
{
	static char *function = "info_handle_section_header_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t<%s>\n",
		 identifier );
	}
	else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
	{
		fprintf(
		 info_handle->notify_stream,
		 "%s\n",
		 description );
	}
	return( 1 );
}

/* Prints a section footer to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_section_footer_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     liberror_error_t **error )
{
	static char *function = "info_handle_section_footer_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t</%s>\n",
		 identifier );
	}
	else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\n" );
	}
	return( 1 );
}

/* Prints a section value string to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_section_value_string_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     size_t identifier_length,
     const char *description,
     size_t description_length,
     const libcstring_system_character_t *value_string,
     liberror_error_t **error )
{
	static char *function = "info_handle_section_value_string_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		if( identifier_length == 12 )
		{
			if( libcstring_narrow_string_compare(
			     identifier,
			     "acquiry_date",
			     12 ) == 0 )
			{
				identifier = "acquisition_date";
			}
		}
		else if( identifier_length == 16 )
		{
			if( libcstring_narrow_string_compare(
			     identifier,
			     "acquiry_software",
			     16 ) == 0 )
			{
				identifier = "acquisition_software";
			}
		}
		else if( identifier_length == 24 )
		{
			if( libcstring_narrow_string_compare(
			     identifier,
			     "acquiry_operating_system",
			     24 ) == 0 )
			{
				identifier = "acquisition_system";
			}
			else if( libcstring_narrow_string_compare(
			          identifier,
			          "acquiry_software_version",
			          24 ) == 0 )
			{
				identifier = "acquisition_version";
			}
		}
		fprintf(
		 info_handle->notify_stream,
		 "\t\t\t<%s>%" PRIs_LIBCSTRING_SYSTEM "</%s>\n",
		 identifier,
		 value_string,
		 identifier );
	}
	else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t%s:",
		 description );

		description_length += 1;

		while( description_length < 24 )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t" );

			description_length += 8;
		}
		fprintf(
		 info_handle->notify_stream,
		 "%" PRIs_LIBCSTRING_SYSTEM "\n",
		 value_string );
	}
	return( 1 );
}

/* Prints a section 32-bit value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_section_value_32bit_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     const char *description,
     size_t description_length,
     uint32_t value_32bit,
     liberror_error_t **error )
{
	static char *function = "info_handle_section_value_32bit_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t\t<%s>%" PRIu32 "</%s>\n",
		 identifier,
		 value_32bit,
		 identifier );
	}
	else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t%s:",
		 description );

		description_length += 1;

		while( description_length < 24 )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t" );

			description_length += 8;
		}
		fprintf(
		 info_handle->notify_stream,
		 "%" PRIu32 "\n",
		 value_32bit );
	}
	return( 1 );
}

/* Prints a section 64-bit value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_section_value_64bit_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     const char *description,
     size_t description_length,
     uint64_t value_64bit,
     liberror_error_t **error )
{
	static char *function = "info_handle_section_value_64bit_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t\t<%s>%" PRIu64 "</%s>\n",
		 identifier,
		 value_64bit,
		 identifier );
	}
	else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t%s:",
		 description );

		description_length += 1;

		while( description_length < 24 )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t" );

			description_length += 8;
		}
		fprintf(
		 info_handle->notify_stream,
		 "%" PRIu64 "\n",
		 value_64bit );
	}
	return( 1 );
}

/* Prints a section 64-bit size value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_section_value_size_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     const char *description,
     size_t description_length,
     size64_t value_size,
     liberror_error_t **error )
{
        libcstring_system_character_t value_size_string[ 16 ];

	static char *function = "info_handle_section_value_size_fprint";
	int result            = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	result = byte_size_string_create(
	          value_size_string,
	          16,
	          value_size,
	          BYTE_SIZE_STRING_UNIT_MEBIBYTE,
	          NULL );

	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		if( result == 1 )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t\t\t<%s>%" PRIs_LIBCSTRING_SYSTEM " (%" PRIu64 " bytes)</%s>\n",
			 identifier,
			 value_size_string,
			 value_size,
			 identifier );
		}
		else
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t\t\t<%s>%" PRIu64 " bytes</%s>\n",
			 identifier,
			 value_size,
			 identifier );
		}
	}
	else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t%s:",
		 description );

		description_length += 1;

		while( description_length < 24 )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t" );

			description_length += 8;
		}
		if( result == 1 )
		{
			fprintf(
			 info_handle->notify_stream,
			 "%" PRIs_LIBCSTRING_SYSTEM " (%" PRIu64 " bytes)\n",
			 value_size_string,
			 value_size );
		}
		else
		{
			fprintf(
			 info_handle->notify_stream,
			 "%" PRIu64 " bytes\n",
			 value_size );
		}
	}
	return( 1 );
}

/* Prints a header value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_header_value_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     size_t identifier_length,
     const char *description,
     size_t description_length,
     liberror_error_t **error )
{
	libcstring_system_character_t header_value[ INFO_HANDLE_VALUE_SIZE ];

	static char *function    = "info_handle_header_value_fprint";
	size_t header_value_size = INFO_HANDLE_VALUE_SIZE;
	int result               = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libewf_handle_get_utf16_header_value(
	          info_handle->input_handle,
	          (uint8_t *) identifier,
	          identifier_length,
	          (uint16_t *) header_value,
	          header_value_size,
	          error );
#else
	result = libewf_handle_get_utf8_header_value(
	          info_handle->input_handle,
	          (uint8_t *) identifier,
	          identifier_length,
	          (uint8_t *) header_value,
	          header_value_size,
	          error );
#endif

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve header value: %s.",
		 function,
		 identifier );

		return( -1 );
	}
	else if( result != 0 )
	{
		if( info_handle_section_value_string_fprint(
		     info_handle,
		     identifier,
		     identifier_length,
		     description,
		     description_length,
		     header_value,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section value string: %s.",
			 function,
			 identifier );

			return( -1 );
		}
	}
	return( 1 );
}

/* Prints the header values to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_header_values_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	char header_value_identifier[ INFO_HANDLE_VALUE_IDENTIFIER_SIZE ];

	const char *description             = NULL;
	static char *function               = "info_handle_header_values_fprint";
	size_t description_length           = 0;
	size_t header_value_identifier_size = INFO_HANDLE_VALUE_IDENTIFIER_SIZE;
	uint32_t header_value_iterator      = 0;
	uint32_t number_of_values           = 0;
	int result                          = 1;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( libewf_handle_set_header_values_date_format(
	     info_handle->input_handle,
	     info_handle->date_format,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set header values date format.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_number_of_header_values(
	     info_handle->input_handle,
	     &number_of_values,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of header values.",
		 function );

		return( -1 );
	}
	if( info_handle_section_header_fprint(
	     info_handle,
	     "acquiry_information",
	     "Acquiry information",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section header: acquiry_information.",
		 function );

		result = -1;
	}
	if( number_of_values == 0 )
	{
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\tNo information found in file.\n" );
		}
	}
	else
	{
		for( header_value_iterator = 0;
		     header_value_iterator < number_of_values;
		     header_value_iterator++ )
		{
			if( libewf_handle_get_header_value_identifier_size(
			     info_handle->input_handle,
			     header_value_iterator,
			     &header_value_identifier_size,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve the header identifier size for index: %" PRIu32 ".",
				 function,
				 header_value_iterator );

				result = -1;

				continue;
			}
			if( header_value_identifier_size > INFO_HANDLE_VALUE_IDENTIFIER_SIZE )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
				 "%s: header identifier size value out of bounds for index: %" PRIu32 ".",
				 function,
				 header_value_iterator );

				result = -1;

				continue;
			}
			if( libewf_handle_get_header_value_identifier(
			     info_handle->input_handle,
			     header_value_iterator,
			     (uint8_t *) header_value_identifier,
			     header_value_identifier_size,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve the header identifier for index: %" PRIu32 ".",
				 function,
				 header_value_iterator );

				result = -1;

				continue;
			}
			description        = NULL;
			description_length = 0;

			if( header_value_identifier_size == 6 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "model",
				     5 ) == 0 )
				{
					description        = "Model";
					description_length = 5;
				}
				else if( libcstring_narrow_string_compare(
					  header_value_identifier,
					  "notes",
					  5 ) == 0 )
				{
					description        = "Notes";
					description_length = 5;
				}
			}
			else if( header_value_identifier_size == 11 )
			{
				/* TODO figure out what this value represents
				 */
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "unknown_dc",
				     10 ) == 0 )
				{
					description        = "Unknown value dc";
					description_length = 16;
				}
			}
			else if( header_value_identifier_size == 12 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "case_number",
				     11 ) == 0 )
				{
					description        = "Case number";
					description_length = 11;
				}
				else if( libcstring_narrow_string_compare(
					  header_value_identifier,
					  "description",
					  11 ) == 0 )
				{
					description        = "Description";
					description_length = 11;
				}
				else if( libcstring_narrow_string_compare(
					  header_value_identifier,
					  "system_date",
					  11 ) == 0 )
				{
					description        = "System date";
					description_length = 11;
				}
			}
			else if( header_value_identifier_size == 13 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "acquiry_date",
				     12 ) == 0 )
				{
					description        = "Acquisition date";
					description_length = 16;
				}
			}
			else if( header_value_identifier_size == 14 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "examiner_name",
				     13 ) == 0 )
				{
					description        = "Examiner name";
					description_length = 13;
				}
				else if( libcstring_narrow_string_compare(
					  header_value_identifier,
					  "serial_number",
					  13 ) == 0 )
				{
					description        = "Serial number";
					description_length = 13;
				}
			}
			else if( header_value_identifier_size == 16 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "evidence_number",
				     15 ) == 0 )
				{
					description        = "Evidence number";
					description_length = 15;
				}
			}
			else if( header_value_identifier_size == 17 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "acquiry_software",
				     16 ) == 0 )
				{
					description        = "Software used";
					description_length = 13;
				}
			}
			else if( header_value_identifier_size == 19 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "process_identifier",
				     18 ) == 0 )
				{
					description        = "Process identifier";
					description_length = 18;
				}
			}
			else if( header_value_identifier_size == 25 )
			{
				if( libcstring_narrow_string_compare(
				     header_value_identifier,
				     "acquiry_operating_system",
				     24 ) == 0 )
				{
					description        = "Operating system used";
					description_length = 21;
				}
				else if( libcstring_narrow_string_compare(
					  header_value_identifier,
					  "acquiry_software_version",
					  24 ) == 0 )
				{
					description        = "Software version used";
					description_length = 21;
				}
			}
			if( description == NULL )
			{
				if( header_value_identifier_size == 8 )
				{
					if( libcstring_narrow_string_compare(
					     header_value_identifier,
					     "extents",
					     7 ) == 0 )
					{
						if( info_handle_header_value_extents_fprint(
						     info_handle,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_RUNTIME,
							 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
							 "%s: unable to print header value: extents.",
							 function );

							result = -1;
						}
					}
				}
				else if( header_value_identifier_size == 9 )
				{
					if( libcstring_narrow_string_compare(
					     header_value_identifier,
					     "password",
					     8 ) == 0 )
					{
						if( info_handle_header_value_password_fprint(
						     info_handle,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_RUNTIME,
							 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
							 "%s: unable to print header value: password.",
							 function );

							result = -1;
						}
					}
				}
				else if( header_value_identifier_size == 18 )
				{
					if( libcstring_narrow_string_compare(
					     header_value_identifier,
					     "compression_level",
					     17 ) == 0 )
					{
						if( info_handle_header_value_compression_level_fprint(
						     info_handle,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_RUNTIME,
							 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
							 "%s: unable to print header value: compression_level.",
							 function );

							result = -1;
						}
					}
				}
				else
				{
					description        = header_value_identifier;
					description_length = header_value_identifier_size - 1;
				}
			}
			if( description != NULL )
			{
				if( info_handle_header_value_fprint(
				     info_handle,
				     header_value_identifier,
				     header_value_identifier_size - 1,
				     description,
				     description_length,
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
					 "%s: unable to print header value: %s.",
					 function,
					 header_value_identifier );

					result = -1;
				}
			}
		}
	}
	if( info_handle_section_footer_fprint(
	     info_handle,
	     "acquiry_information",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section footer: acquiry_information.",
		 function );

		result = -1;
	}
	return( result );
}

/* Prints the password header value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_header_value_password_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	libcstring_system_character_t header_value[ INFO_HANDLE_VALUE_SIZE ];

	static char *function    = "info_handle_header_value_password_fprint";
	size_t header_value_size = INFO_HANDLE_VALUE_SIZE;
	int result               = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libewf_handle_get_utf16_header_value(
	          info_handle->input_handle,
	          (uint8_t *) "password",
	          8,
	          (uint16_t *) header_value,
	          header_value_size,
	          error );
#else
	result = libewf_handle_get_utf8_header_value(
	          info_handle->input_handle,
	          (uint8_t *) "password",
	          8,
	          (uint8_t *) header_value,
	          header_value_size,
	          error );
#endif
	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve header value: password.",
		 function );

		return( -1 );
	}
	else if( result == 0 )
	{
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\tPassword:\t\tN/A\n" );
		}
	}
	else
	{
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t\t\t<password>%" PRIs_LIBCSTRING_SYSTEM "</password>\n",
			 header_value );
		}
		else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\tPassword:\t\t(hash: %" PRIs_LIBCSTRING_SYSTEM ")\n",
			 header_value );
		}
	}
	return( 1 );
}

/* Prints the compression level header value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_header_value_compression_level_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	libcstring_system_character_t header_value[ INFO_HANDLE_VALUE_SIZE ];

	const libcstring_system_character_t *value_string = NULL;
	static char *function                             = "info_handle_header_value_compression_level_fprint";
	size_t header_value_size                          = INFO_HANDLE_VALUE_SIZE;
	int result                                        = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libewf_handle_get_utf16_header_value(
	          info_handle->input_handle,
	          (uint8_t *) "compression_level",
	          16,
	          (uint16_t *) header_value,
	          header_value_size,
	          error );
#else
	result = libewf_handle_get_utf8_header_value(
	          info_handle->input_handle,
	          (uint8_t *) "compression_level",
	          16,
	          (uint8_t *) header_value,
	          header_value_size,
	          error );
#endif
	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve header value: compression_level.",
		 function );

		return( -1 );
	}
	else if( result != 0 )
	{
		if( libcstring_system_string_compare(
		     header_value,
		     _LIBCSTRING_SYSTEM_STRING( LIBEWF_COMPRESSION_LEVEL_NONE ),
		     1 ) == 0 )
		{
			value_string = _LIBCSTRING_SYSTEM_STRING( "no compression" );
		}
		else if( libcstring_system_string_compare(
			  header_value,
			  _LIBCSTRING_SYSTEM_STRING( LIBEWF_COMPRESSION_LEVEL_FAST ),
			  1 ) == 0 )
		{
			value_string = _LIBCSTRING_SYSTEM_STRING( "good (fast) compression" );
		}
		else if( libcstring_system_string_compare(
			  header_value,
			  _LIBCSTRING_SYSTEM_STRING( LIBEWF_COMPRESSION_LEVEL_BEST ),
			  1 ) == 0 )
		{
			value_string = _LIBCSTRING_SYSTEM_STRING( "best compression" );
		}
		else
		{
			value_string = _LIBCSTRING_SYSTEM_STRING( "unknown compression" );
		}
		if( info_handle_section_value_string_fprint(
		     info_handle,
		     "compression_level",
		     16,
		     "Compression level",
		     16,
		     value_string,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section value string: compression_level.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Prints the extents header value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_header_value_extents_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	libcstring_system_character_t header_value[ INFO_HANDLE_VALUE_SIZE ];

	libsystem_split_values_t *extents_elements = NULL;
	static char *function                      = "info_handle_header_value_extents_fprint";
	size_t header_value_length                 = 0;
	size_t header_value_size                   = INFO_HANDLE_VALUE_SIZE;
	int extents_element_iterator               = 0;
	int result                                 = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libewf_handle_get_utf16_header_value(
	          info_handle->input_handle,
	          (uint8_t *) "extents",
	          7,
	          (uint16_t *) header_value,
	          header_value_size,
	          error );
#else
	result = libewf_handle_get_utf8_header_value(
	          info_handle->input_handle,
	          (uint8_t *) "extents",
	          7,
	          (uint8_t *) header_value,
	          header_value_size,
	          error );
#endif
	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve header value: extents.",
		 function );

		goto on_error;
	}
	else if( result != 0 )
	{
		/* Want the effective length of the string
		 */
		header_value_length = libcstring_system_string_length(
		                       header_value );

		if( libsystem_split_values_parse_string(
		     &extents_elements,
		     header_value,
		     header_value_length,
		     (libcstring_system_character_t) ' ',
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to split header value into elements.",
			 function );

			goto on_error;
		}
		if( ( extents_elements->number_of_values % 4 ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported number of extents elements in header value.",
			 function );

			goto on_error;
		}
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\tExtents:\t\t%" PRIs_LIBCSTRING_SYSTEM "\n",
			 extents_elements->values[ 0 ] );
		}

		if( extents_elements->number_of_values > 1 )
		{
			if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\t\t\t<extents>\n" );
			}
			for( extents_element_iterator = 1;
			     extents_element_iterator < extents_elements->number_of_values;
			     extents_element_iterator += 4 )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\t\t\t\t" );

				if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
				{
					fprintf(
					 info_handle->notify_stream,
					 "<extent>" );
				}
				fprintf(
				 info_handle->notify_stream,
				 "%" PRIs_LIBCSTRING_SYSTEM " %" PRIs_LIBCSTRING_SYSTEM " %" PRIs_LIBCSTRING_SYSTEM " %" PRIs_LIBCSTRING_SYSTEM "",
				 extents_elements->values[ extents_element_iterator ],
				 extents_elements->values[ extents_element_iterator + 1 ],
				 extents_elements->values[ extents_element_iterator + 2 ],
				 extents_elements->values[ extents_element_iterator + 3 ] );

				if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
				{
					fprintf(
					 info_handle->notify_stream,
					 "</extent>" );
				}
				fprintf(
				 info_handle->notify_stream,
				 "\n" );
			}
			if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\t\t\t</extents>\n" );
			}
		}
		if( libsystem_split_values_free(
		     &extents_elements,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free split date time elements.",
			 function );

			goto on_error;
		}
	}
	return( 1 );

on_error:
	if( extents_elements != NULL )
	{
		libsystem_split_values_free(
		 &extents_elements,
		 NULL );
	}
	return( -1 );
}

/* Prints the media information to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_media_information_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
        libcstring_system_character_t guid_string[ GUID_STRING_SIZE ];
        uint8_t guid[ GUID_SIZE ];

	const libcstring_system_character_t *value_string = NULL;
	static char *function                             = "info_handle_media_information_fprint";
	size64_t media_size                               = 0;
	uint64_t value_64bit                              = 0;
	uint32_t value_32bit                              = 0;
	uint8_t compression_flags                         = 0;
	uint8_t media_type                                = 0;
	uint8_t media_flags                               = 0;
	uint8_t format                                    = 0;
	int8_t compression_level                          = 0;
	int result                                        = 1;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( info_handle_section_header_fprint(
	     info_handle,
	     "ewf_information",
	     "EWF information",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section header: ewf_information.",
		 function );

		result = -1;
	}
	if( libewf_handle_get_format(
	     info_handle->input_handle,
	     &format,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve format.",
		 function );

		return( -1 );
	}
	switch( format )
	{
		case LIBEWF_FORMAT_EWF:
			value_string = _LIBCSTRING_SYSTEM_STRING( "original EWF" );
			break;

		case LIBEWF_FORMAT_SMART:
			value_string = _LIBCSTRING_SYSTEM_STRING( "SMART" );
			break;

		case LIBEWF_FORMAT_FTK:
			value_string = _LIBCSTRING_SYSTEM_STRING( "FTK Imager" );
			break;

		case LIBEWF_FORMAT_ENCASE1:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EnCase 1" );
			break;

		case LIBEWF_FORMAT_ENCASE2:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EnCase 2" );
			break;

		case LIBEWF_FORMAT_ENCASE3:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EnCase 3" );
			break;

		case LIBEWF_FORMAT_ENCASE4:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EnCase 4" );
			break;

		case LIBEWF_FORMAT_ENCASE5:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EnCase 5" );
			break;

		case LIBEWF_FORMAT_ENCASE6:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EnCase 6" );
			break;

		case LIBEWF_FORMAT_LINEN5:
			value_string = _LIBCSTRING_SYSTEM_STRING( "linen 5" );
			break;

		case LIBEWF_FORMAT_LINEN6:
			value_string = _LIBCSTRING_SYSTEM_STRING( "linen 6" );
			break;

		case LIBEWF_FORMAT_EWFX:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EWFX (extended EWF)" );
			break;

		case LIBEWF_FORMAT_LVF:
			value_string = _LIBCSTRING_SYSTEM_STRING( "EnCase Logical File Evidence (LVF)" );
			break;

		case LIBEWF_FORMAT_UNKNOWN:
		default:
			value_string = _LIBCSTRING_SYSTEM_STRING( "unknown" );
			break;

	}
	if( info_handle_section_value_string_fprint(
	     info_handle,
	     "file_format",
	     11,
	     "File format",
	     11,
	     value_string,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section value string: file_format.",
		 function );

		result = -1;
	}
	if( ( format == LIBEWF_FORMAT_ENCASE5 )
	 || ( format == LIBEWF_FORMAT_ENCASE6 )
	 || ( format == LIBEWF_FORMAT_LINEN5 )
	 || ( format == LIBEWF_FORMAT_LINEN6 )
	 || ( format == LIBEWF_FORMAT_EWFX ) )
	{
		if( libewf_handle_get_sectors_per_chunk(
		     info_handle->input_handle,
		     &value_32bit,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve sectors per chunk.",
			 function );

			result = -1;
		}
		else
		{
			if( info_handle_section_value_32bit_fprint(
			     info_handle,
			     "sectors_per_chunk",
			     "Sectors per chunk",
			     16,
			     value_32bit,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print section 32-bit value: sectors_per_chunk.",
				 function );

				result = -1;
			}
		}
		if( libewf_handle_get_error_granularity(
		     info_handle->input_handle,
		     &value_32bit,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve error granularity.",
			 function );

			result = -1;
		}
		else
		{
			if( info_handle_section_value_32bit_fprint(
			     info_handle,
			     "error_granularity",
			     "Error granularity",
			     16,
			     value_32bit,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print section 32-bit value: error_granularity.",
				 function );

				result = -1;
			}
		}
		if( libewf_handle_get_compression_values(
		     info_handle->input_handle,
		     &compression_level,
		     &compression_flags,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve compression values.",
			 function );

			result = -1;
		}
		else
		{
			if( compression_level == LIBEWF_COMPRESSION_NONE )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "no compression" );
			}
			else if( compression_level == LIBEWF_COMPRESSION_FAST )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "good (fast) compression" );
			}
			else if( compression_level == LIBEWF_COMPRESSION_BEST )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "best compression" );
			}
			else
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "unknown compression" );
			}
			if( info_handle_section_value_string_fprint(
			     info_handle,
			     "compression_level",
			     16,
			     "Compression level",
			     16,
			     value_string,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print section value string: compression_level.",
				 function );

				result = -1;
			}
		}
		if( libewf_handle_get_guid(
		     info_handle->input_handle,
		     guid,
		     GUID_SIZE,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve GUID.",
			 function );

			result = -1;
		}
		else
		{
			if( guid_to_string(
			     guid,
			     GUID_SIZE,
			     _BYTE_STREAM_ENDIAN_LITTLE,
			     guid_string,
			     GUID_STRING_SIZE,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to create GUID string.",
				 function );

				result = -1;
			}
			else
			{
				if( info_handle_section_value_string_fprint(
				     info_handle,
				     "guid",
				     4,
				     "GUID",
				     4,
				     guid_string,
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
					 "%s: unable to print section value string: guid.",
					 function );

					result = -1;
				}
			}
		}
	}
	if( info_handle_section_footer_fprint(
	     info_handle,
	     "ewf_information",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section footer: ewf_information.",
		 function );

		result = -1;
	}
	if( info_handle_section_header_fprint(
	     info_handle,
	     "media_information",
	     "Media information",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section header: media_information.",
		 function );

		result = -1;
	}
	if( ( format != LIBEWF_FORMAT_EWF )
	 && ( format != LIBEWF_FORMAT_SMART ) )
	{
		if( libewf_handle_get_media_type(
		     info_handle->input_handle,
		     &media_type,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve media type.",
			 function );

			result = -1;
		}
		else
		{
			if( media_type == LIBEWF_MEDIA_TYPE_REMOVABLE )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "removable disk" );
			}
			else if( media_type == LIBEWF_MEDIA_TYPE_FIXED )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "fixed disk" );
			}
			else if( media_type == LIBEWF_MEDIA_TYPE_SINGLE_FILES )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "single files" );
			}
			else if( media_type == LIBEWF_MEDIA_TYPE_OPTICAL )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "optical disk (CD/DVD/BD)" );
			}
			else if( media_type == LIBEWF_MEDIA_TYPE_MEMORY )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "memory (RAM)" );
			}
			else
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "unknown" );
			}
			if( info_handle_section_value_string_fprint(
			     info_handle,
			     "media_type",
			     10,
			     "Media type",
			     10,
			     value_string,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print section value string: media_type.",
				 function );

				result = -1;
			}
		}
		if( libewf_handle_get_media_flags(
		     info_handle->input_handle,
		     &media_flags,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve media flags.",
			 function );

			result = -1;
		}
		else
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\tMedia flags:\t\t0x%02" PRIx8 "\n",
				 media_flags );
			}
#endif
			if( ( media_flags & LIBEWF_MEDIA_FLAG_PHYSICAL ) != 0 )
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "yes" );
			}
			else
			{
				value_string = _LIBCSTRING_SYSTEM_STRING( "no" );
			}
			if( info_handle_section_value_string_fprint(
			     info_handle,
			     "is_physical",
			     10,
			     "Is physical",
			     10,
			     value_string,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print section value string: media_type.",
				 function );

				result = -1;
			}
			if( ( media_flags & LIBEWF_MEDIA_FLAG_FASTBLOC ) != 0 )
			{
				if( info_handle_section_value_string_fprint(
				     info_handle,
				     "write_blocked",
				     13,
				     "Write blocked",
				     13,
				     _LIBCSTRING_SYSTEM_STRING( "Fastbloc" ),
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
					 "%s: unable to print section value string: write_blocked.",
					 function );

					result = -1;
				}
			}
			if( ( media_flags & LIBEWF_MEDIA_FLAG_TABLEAU ) != 0 )
			{
				if( info_handle_section_value_string_fprint(
				     info_handle,
				     "write_blocked",
				     13,
				     "Write blocked",
				     13,
				     _LIBCSTRING_SYSTEM_STRING( "Tableau" ),
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
					 "%s: unable to print section value string: write_blocked.",
					 function );

					result = -1;
				}
			}
		}
	}
	if( format != LIBEWF_FORMAT_LVF )
	{
		if( libewf_handle_get_bytes_per_sector(
		     info_handle->input_handle,
		     &value_32bit,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve bytes per sector.",
			 function );

			result = -1;
		}
		else
		{
			if( info_handle_section_value_32bit_fprint(
			     info_handle,
			     "bytes_per_sector",
			     "Bytes per sector",
			     16,
			     value_32bit,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print section 32-bit value: bytes_per_sector.",
				 function );

				result = -1;
			}
		}
		if( libewf_handle_get_number_of_sectors(
		     info_handle->input_handle,
		     &value_64bit,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve number of sectors.",
			 function );

			result = -1;
		}
		else
		{
			if( info_handle_section_value_64bit_fprint(
			     info_handle,
			     "number_of_sectors",
			     "Number of sectors",
			     17,
			     value_64bit,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print section 64-bit value: number_of_sectors.",
				 function );

				result = -1;
			}
		}
	}
	if( libewf_handle_get_media_size(
	     info_handle->input_handle,
	     &media_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve media size.",
		 function );

		result = -1;
	}
	else
	{
		if( info_handle_section_value_size_fprint(
		     info_handle,
		     "media_size",
		     "Media size",
		     10,
		     media_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section 64-bit value: media_size.",
			 function );

			result = -1;
		}
	}
	if( info_handle_section_footer_fprint(
	     info_handle,
	     "media_information",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section footer: media_information.",
		 function );

		result = -1;
	}
	return( result );
}

/* Prints a hash value to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_hash_value_fprint(
     info_handle_t *info_handle,
     const char *identifier,
     size_t identifier_length,
     liberror_error_t **error )
{
	libcstring_system_character_t hash_value[ INFO_HANDLE_VALUE_SIZE ];

	static char *function  = "info_handle_hash_value_fprint";
	size_t hash_value_size = INFO_HANDLE_VALUE_SIZE;
	int result             = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libewf_handle_get_utf16_hash_value(
	          info_handle->input_handle,
	          (uint8_t *) identifier,
	          identifier_length,
	          (uint16_t *) hash_value,
	          hash_value_size,
	          error );
#else
	result = libewf_handle_get_utf8_hash_value(
	          info_handle->input_handle,
	          (uint8_t *) identifier,
	          identifier_length,
	          (uint8_t *) hash_value,
	          hash_value_size,
	          error );
#endif

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve hash value: %s.",
		 function,
		 identifier );

		return( -1 );
	}
	else if( result != 0 )
	{
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
		{
			if( identifier_length == 3 )
			{
				if( libcstring_narrow_string_compare(
				     identifier,
				     "MD5",
				     3 ) == 0 )
				{
					identifier = "md5";
				}
			}
			else if( identifier_length == 4 )
			{
				if( libcstring_narrow_string_compare(
				     identifier,
				     "SHA1",
				     4 ) == 0 )
				{
					identifier = "sha1";
				}
			}
			fprintf(
			 info_handle->notify_stream,
			 "\t\t<hashdigest type=\"%s\" coding=\"base16\">%" PRIs_LIBCSTRING_SYSTEM "</hashdigest>\n",
			 identifier,
			 hash_value );
		}
		else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t%s:\t\t\t%" PRIs_LIBCSTRING_SYSTEM "\n",
			 identifier,
			 hash_value );
		}
	}
	return( 1 );
}

/* Prints the hash values to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_hash_values_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	char hash_value_identifier[ INFO_HANDLE_VALUE_IDENTIFIER_SIZE ];

	static char *function             = "info_handle_hash_values_fprint";
	size_t hash_value_identifier_size = INFO_HANDLE_VALUE_IDENTIFIER_SIZE;
	uint32_t number_of_values         = 0;
	uint32_t hash_value_iterator      = 0;
	uint8_t print_section_header      = 1;
	int result                        = 1;

#if defined( USE_LIBEWF_GET_MD5_HASH )
	digest_hash_t md5_hash[ DIGEST_HASH_SIZE_MD5 ];

	libcstring_system_character_t *stored_md5_hash_string = NULL;
#endif

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
#if defined( USE_LIBEWF_GET_MD5_HASH )
	result = libewf_handle_get_md5_hash(
		  handle,
		  md5_hash,
		  DIGEST_HASH_SIZE_MD5,
	          error );

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve MD5 hash.",
		 function );

		return;
	}
	else if( result == 1 )
	{
		stored_md5_hash_string = (libcstring_system_character_t *) memory_allocate(
		                                                            sizeof( libcstring_system_character_t ) * DIGEST_HASH_STRING_SIZE_MD5 );

		if( stored_md5_hash_string == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create MD5 hash string.",
			 function );

			return;
		}
		if( digest_hash_copy_to_string(
		     md5_hash,
		     DIGEST_HASH_SIZE_MD5,
		     stored_md5_hash_string,
		     DIGEST_HASH_STRING_SIZE_MD5,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set MD5 hash string.",
			 function );

			memory_free(
			 stored_md5_hash_string );

			return;
		}
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			if( print_section_header != 0 )
			{
				if( info_handle_section_header_fprint(
				     info_handle,
				     "digest_hash_information",
				     "Digest hash information",
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
					 "%s: unable to print section header: digest_hash_information.",
					 function );

					result = -1;
				}
				print_section_header = 0;
			}
		}
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t\t<hashdigest type=\"md5\" coding=\"base16\">%" PRIs_LIBCSTRING_SYSTEM "</hashdigest>\n",
			 stored_md5_hash_string );
		}
		else if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\tMD5:\t\t\t%" PRIs_LIBCSTRING_SYSTEM "\n",
			 stored_md5_hash_string );
		}
		memory_free(
		 stored_md5_hash_string );
	}
#endif
	if( libewf_handle_get_number_of_hash_values(
	     info_handle->input_handle,
	     &number_of_values,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of hash values.",
		 function );

		return( -1 );
	}
	for( hash_value_iterator = 0;
	     hash_value_iterator < number_of_values;
	     hash_value_iterator++ )
	{
		if( libewf_handle_get_hash_value_identifier_size(
		     info_handle->input_handle,
		     hash_value_iterator,
		     &hash_value_identifier_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve the hash value identifier size for index: %" PRIu32 ".",
			 function,
			 hash_value_iterator );

			result = -1;

			continue;
		}
		if( hash_value_identifier_size > INFO_HANDLE_VALUE_IDENTIFIER_SIZE )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: hash value identifier size value out of bounds for index: %" PRIu32 ".",
			 function,
			 hash_value_iterator );

			result = -1;

			continue;
		}
		if( libewf_handle_get_hash_value_identifier(
		     info_handle->input_handle,
		     hash_value_iterator,
		     (uint8_t *) hash_value_identifier,
		     hash_value_identifier_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve the hash value identifier for index: %" PRIu32 ".",
			 function,
			 hash_value_iterator );

			result = -1;

			continue;
		}
#if defined( USE_LIBEWF_GET_MD5_HASH )
		if( hash_value_identifier_size == 4 )
		{
			if( narrow_string_compare(
			     hash_value_identifier,
			     "MD5",
			     3 ) == 0 )
			{
				continue;
			}
		}
#endif
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			if( print_section_header != 0 )
			{
				if( info_handle_section_header_fprint(
				     info_handle,
				     "digest_hash_information",
				     "Digest hash information",
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
					 "%s: unable to print section header: digest_hash_information.",
					 function );

					result = -1;
				}
				print_section_header = 0;
			}
		}
		if( info_handle_hash_value_fprint(
		     info_handle,
		     hash_value_identifier,
		     hash_value_identifier_size - 1,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print hash value: %s.",
			 function,
			 hash_value_identifier );

			result = -1;
		}
	}
	if( print_section_header == 0 )
	{
		if( info_handle_section_footer_fprint(
		     info_handle,
		     "digest_hash_information.",
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section footer: digest_hash_information.",
			 function );

			result = -1;
		}
	}
	return( result );
}

/* Prints the acquiry errors to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_acquiry_errors_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	static char *function      = "info_handle_acquiry_errors_fprint";
	uint64_t start_sector      = 0;
	uint64_t number_of_sectors = 0;
	uint32_t bytes_per_sector  = 0;
	uint32_t number_of_errors  = 0;
	uint32_t error_iterator    = 0;
	int result                 = 1;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_bytes_per_sector(
	     info_handle->input_handle,
	     &bytes_per_sector,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve bytes per sector.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_number_of_acquiry_errors(
	     info_handle->input_handle,
	     &number_of_errors,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of acquiry errors.",
		 function );

		return( -1 );
	}
	if( number_of_errors > 0 )
	{
		if( info_handle_section_header_fprint(
		     info_handle,
		     "acquisition_read_errors",
		     "Read errors during acquiry",
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section header: acquisition_read_errors.",
			 function );

			result = -1;
		}
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\ttotal number: %" PRIu32 "\n",
			 number_of_errors );
		}
		for( error_iterator = 0;
		     error_iterator < number_of_errors;
		     error_iterator++ )
		{
			if( libewf_handle_get_acquiry_error(
			     info_handle->input_handle,
			     error_iterator,
			     &start_sector,
			     &number_of_sectors,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve the acquiry error: %" PRIu32 ".",
				 function,
				 error_iterator );

				start_sector      = 0;
				number_of_sectors = 0;

				result = -1;
			}
			if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\t\t\t<run image_offset=\"%" PRIu64 "\" len=\"%" PRIu64 "\"/>\n",
				 start_sector * bytes_per_sector,
				 number_of_sectors * bytes_per_sector );
			}
			if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\tat sector(s): %" PRIu64 " - %" PRIu64 " number: %" PRIu64 "\n",
				 start_sector,
				 start_sector + number_of_sectors,
				 number_of_sectors );
			}
		}
		if( info_handle_section_footer_fprint(
		     info_handle,
		     "acquisition_read_errors",
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section footer: acquisition_read_errors.",
			 function );

			result = -1;
		}
	}
	return( result );
}

/* Prints the sessions to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_sessions_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	static char *function       = "info_handle_sessions_fprint";
	uint64_t start_sector       = 0;
	uint64_t number_of_sectors  = 0;
	uint32_t bytes_per_sector   = 0;
	uint32_t number_of_sessions = 0;
	uint32_t session_iterator   = 0;
	int result                  = 1;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_bytes_per_sector(
	     info_handle->input_handle,
	     &bytes_per_sector,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve bytes per sector.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_number_of_sessions(
	     info_handle->input_handle,
	     &number_of_sessions,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of sessions.",
		 function );

		return( -1 );
	}
	if( number_of_sessions > 0 )
	{
		if( info_handle_section_header_fprint(
		     info_handle,
		     "sessions",
		     "Sessions",
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section header: sessions.",
			 function );

			result = -1;
		}
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\ttotal number: %" PRIu32 "\n",
			 number_of_sessions );
		}
		for( session_iterator = 0;
		     session_iterator < number_of_sessions;
		     session_iterator++ )
		{
			if( libewf_handle_get_session(
			     info_handle->input_handle,
			     session_iterator,
			     &start_sector,
			     &number_of_sectors,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve the session: %" PRIu32 ".",
				 function,
				 session_iterator );

				start_sector      = 0;
				number_of_sectors = 0;

				result = -1;
			}
			if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\t\t\t<run image_offset=\"%" PRIu64 "\" len=\"%" PRIu64 "\"/>\n",
				 start_sector * bytes_per_sector,
				 number_of_sectors * bytes_per_sector );
			}
			if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
			{
				fprintf(
				 info_handle->notify_stream,
				 "\tat sector(s): %" PRIu64 " - %" PRIu64 " number: %" PRIu64 "\n",
				 start_sector,
				 start_sector + number_of_sectors,
				 number_of_sectors );
			}
		}
		if( info_handle_section_footer_fprint(
		     info_handle,
		     "sessions",
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print section footer: sessions.",
			 function );

			result = -1;
		}
	}
	return( result );
}

/* Prints the single files to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_single_files_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	libewf_file_entry_t *file_entry = NULL;
	static char *function           = "info_handle_single_files_fprint";
	int result                      = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( info_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid info handle - missing input handle.",
		 function );

		return( -1 );
	}
	result = libewf_handle_get_root_file_entry(
	          info_handle->input_handle,
	          &file_entry,
	          error );

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve root file entry.",
		 function );

		return( -1 );
	}
	else if( result == 0 )
	{
		return( 1 );
	}
	if( info_handle_section_header_fprint(
	     info_handle,
	     "single_files",
	     "Single files",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section header: single_files.",
		 function );

		result = -1;
	}
	if( info_handle_file_entry_fprint(
	     info_handle,
	     file_entry,
	     0,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print root file entry.",
		 function );

		result = -1;
	}
	if( info_handle_section_footer_fprint(
	     info_handle,
	     "single_files",
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print section footer: single_files.",
		 function );

		result = -1;
	}
	if( libewf_file_entry_free(
	     &file_entry,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free root file entry.",
		 function );

		result = -1;
	}
	return( result );
}

/* Prints the (single) file entry to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_file_entry_fprint(
     info_handle_t *info_handle,
     libewf_file_entry_t *file_entry,
     int level,
     liberror_error_t **error )
{
	libcstring_system_character_t *name = NULL;
	libewf_file_entry_t *sub_file_entry = NULL;
	static char *function               = "info_handle_file_entry_fprint";
	size_t name_size                    = 0;
	int iterator                        = 0;
	int number_of_sub_file_entries      = 0;
	int result                          = 0;

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t\t<file_entry name=\"" );
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libewf_file_entry_get_utf16_name_size(
	          file_entry,
	          &name_size,
	          error );
#else
	result = libewf_file_entry_get_utf8_name_size(
	          file_entry,
	          &name_size,
	          error );
#endif
	if( result != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the name.",
		 function );

		return( -1 );
	}
	if( name_size > 0 )
	{
		name = (libcstring_system_character_t *) memory_allocate(
		                                          sizeof( libcstring_system_character_t ) * name_size );

		if( name == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create name.",
			 function );

			return( -1 );
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libewf_file_entry_get_utf16_name(
		          file_entry,
		          (uint16_t *) name,
		          name_size,
		          error );
#else
		result = libewf_file_entry_get_utf8_name(
		          file_entry,
		          (uint8_t *) name,
		          name_size,
		          error );
#endif
		if( result != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve the name.",
			 function );

			memory_free(
			 name );

			return( -1 );
		}
		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\t" );

			for( iterator = 1;
			     iterator < level;
			     iterator++ )
			{
				fprintf(
				 info_handle->notify_stream,
				 " " );
			}
		}
		fprintf(
		 info_handle->notify_stream,
		 "%" PRIs_LIBCSTRING_SYSTEM "",
		 name );

		if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_TEXT )
		{
			fprintf(
			 info_handle->notify_stream,
			 "\n" );
		}
		memory_free(
		 name );
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\">\n" );
	}
	if( libewf_file_entry_get_number_of_sub_file_entries(
	     file_entry,
	     &number_of_sub_file_entries,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of sub file entries.",
		 function );

		return( -1 );
	}
	for( iterator = 0;
	     iterator < number_of_sub_file_entries;
	     iterator++ )
	{
		if( libewf_file_entry_get_sub_file_entry(
		     file_entry,
		     iterator,
		     &sub_file_entry,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to free retrieve sub file entry: %d.",
			 function,
			 iterator + 1 );

			return( -1 );
		}
		if( info_handle_file_entry_fprint(
		     info_handle,
		     sub_file_entry,
		     level + 1,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to print sub file entry: %d.",
			 function,
			 iterator + 1 );

			libewf_file_entry_free(
			 &sub_file_entry,
			 NULL );

			return( -1 );
		}
		if( libewf_file_entry_free(
		     &sub_file_entry,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free sub file entry: %d.",
			 function,
			 iterator + 1 );

			return( -1 );
		}
	}
	if( info_handle->output_format == INFO_HANDLE_OUTPUT_FORMAT_DFXML )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t\t</file_entry>\n" );
	}
	return( 1 );
}

/* Prints the DFXML header to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_dfxml_header_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	static char *function = "info_handle_dfxml_header_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );

	/* TODO what about DTD or XSD ? */

	fprintf(
	 info_handle->notify_stream,
	 "<ewfobjects version=\"0.1\">\n" );

	fprintf(
	 info_handle->notify_stream,
	 "\t<metadata xmlns=\"http://libewf.sourceforge.net/\"\n"
	 "\t          xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
	 "\t          xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
	 "\t\t<dc:type>Disk Image</dc:type>\n"
	 "\t</metadata>\n" );

	fprintf(
	 info_handle->notify_stream,
	 "\t<creator>\n"
	 "\t\t<program>ewfinfo</program>\n"
	 "\t\t<version>%s</version>\n",
	 LIBEWF_VERSION_STRING );

	if( info_handle_dfxml_build_environment_fprint(
	     info_handle,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print build environment.",
		 function );

		return( -1 );
	}
	if( info_handle_dfxml_execution_environment_fprint(
	     info_handle,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to print execution environment.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "\t</creator>\n"
	 "\t<ewfinfo>\n" );

	return( 1 );
}

/* Prints the DFXML build environment to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_dfxml_build_environment_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	static char *function = "info_handle_dfxml_build_environment_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "\t\t<build_environment>\n" );

	/* TODO add MSC and BORLANC support */
#if defined( _MSC_VER )

#elif defined( __BORLANDC__ )

#elif defined( __GNUC__ ) && defined( __GNUC_MINOR__ )
#if defined( __MINGW64_VERSION_MAJOR ) && defined( __MINGW64_VERSION_MINOR )
	fprintf(
	 info_handle->notify_stream,
	 "\t\t\t<compiler>MinGW64 %d.%d</compiler>\n",
	 __MINGW64_VERSION_MAJOR,
	 __MINGW64_VERSION_MINOR );
#elif defined( __MINGW32_MAJOR_VERSION ) && defined( __MINGW32_MINOR_VERSION )
	fprintf(
	 info_handle->notify_stream,
	 "\t\t\t<compiler>MinGW32 %d.%d</compiler>\n",
	 __MINGW32_MAJOR_VERSION,
	 __MINGW32_MINOR_VERSION );
#endif
	fprintf(
	 info_handle->notify_stream,
	 "\t\t\t<compiler>GCC %d.%d</compiler>\n",
	 __GNUC__,
	 __GNUC_MINOR__ );
#endif		
	fprintf(
	 info_handle->notify_stream,
	 "\t\t\t<compilation_date>" __DATE__ " " __TIME__ "</compilation_date>\n" );

	fprintf(
	 info_handle->notify_stream,
	 "\t\t\t<library name=\"libewf\" version=\"%s\"/>\n",
	 LIBEWF_VERSION_STRING );

	/* TODO add other libraries
	 */

	fprintf(
	 info_handle->notify_stream,
	 "\t\t</build_environment>\n" );

	return( 1 );
}

/* Prints the DFXML execution environment to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_dfxml_execution_environment_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
#if defined( HAVE_UNAME ) && !defined( WINAPI )
	struct utsname utsname_buffer;
#endif

	static char *function = "info_handle_dfxml_execution_environment_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	/* TODO what about execution environment on other platforms ? */

#if defined( HAVE_UNAME )
	if( uname(
	     &utsname_buffer ) == 0 )
	{
		fprintf(
		 info_handle->notify_stream,
		 "\t\t<execution_environment>\n"
		 "\t\t\t<os_sysname>%s</os_sysname>\n"
		 "\t\t\t<os_release>%s</os_release>\n"
		 "\t\t\t<os_version>%s</os_version>\n"
		 "\t\t\t<host>%s</host>\n"
		 "\t\t\t<arch>%s</arch>\n"
		 "\t\t</execution_environment>\n",
		 utsname_buffer.sysname,
		 utsname_buffer.release,
		 utsname_buffer.version,
		 utsname_buffer.nodename,
		 utsname_buffer.machine );
	}
#endif
	/* TODO
	 * <command_line> X </command_line>
	 * <uid> getuid() </uid>
	 * <username> getpwuid( getuid() )->pw_name </username>
	 */

	return( 1 );
}

/* Prints the DFXML footer to a stream
 * Returns 1 if successful or -1 on error
 */
int info_handle_dfxml_footer_fprint(
     info_handle_t *info_handle,
     liberror_error_t **error )
{
	static char *function = "info_handle_dfxml_footer_fprint";

	if( info_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid info handle.",
		 function );

		return( -1 );
	}
	fprintf(
	 info_handle->notify_stream,
	 "\t</ewfinfo>\n"
	 "</ewfobjects>\n"
	 "\n" );

	return( 1 );
}

