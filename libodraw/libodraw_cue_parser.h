/* A Bison parser, made by GNU Bison 3.5.0.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_LIBODRAW_CUE_SCANNER_D_CODE_B_VOUND_GITHUB_LIBEWF_EXP_LIBODRAW_LIBODRAW_CUE_PARSER_H_INCLUDED
# define YY_LIBODRAW_CUE_SCANNER_D_CODE_B_VOUND_GITHUB_LIBEWF_EXP_LIBODRAW_LIBODRAW_CUE_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int libodraw_cue_scanner_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    CUE_END_OF_LINE = 258,
    CUE_SEMI_COLON = 259,
    CUE_2DIGIT = 260,
    CUE_CATALOG_NUMBER = 261,
    CUE_ISRC_CODE = 262,
    CUE_KEYWORD_STRING = 263,
    CUE_MSF = 264,
    CUE_STRING = 265,
    CUE_CATALOG = 266,
    CUE_CD_DA = 267,
    CUE_CD_ROM = 268,
    CUE_CD_ROM_XA = 269,
    CUE_CD_TEXT = 270,
    CUE_CDTEXTFILE = 271,
    CUE_COPY = 272,
    CUE_DATAFILE = 273,
    CUE_FLAGS = 274,
    CUE_FOUR_CHANNEL_AUDIO = 275,
    CUE_FILE = 276,
    CUE_INDEX = 277,
    CUE_ISRC = 278,
    CUE_NO_COPY = 279,
    CUE_NO_PRE_EMPHASIS = 280,
    CUE_POSTGAP = 281,
    CUE_PRE_EMPHASIS = 282,
    CUE_PREGAP = 283,
    CUE_REMARK = 284,
    CUE_TRACK = 285,
    CUE_TWO_CHANNEL_AUDIO = 286,
    CUE_CDTEXT_ARRANGER = 287,
    CUE_CDTEXT_COMPOSER = 288,
    CUE_CDTEXT_DISC_ID = 289,
    CUE_CDTEXT_GENRE = 290,
    CUE_CDTEXT_MESSAGE = 291,
    CUE_CDTEXT_PERFORMER = 292,
    CUE_CDTEXT_SIZE_INFO = 293,
    CUE_CDTEXT_SONGWRITER = 294,
    CUE_CDTEXT_TITLE = 295,
    CUE_CDTEXT_TOC_INFO1 = 296,
    CUE_CDTEXT_TOC_INFO2 = 297,
    CUE_CDTEXT_UPC_EAN = 298,
    CUE_REMARK_LEAD_OUT = 299,
    CUE_REMARK_ORIGINAL_MEDIA_TYPE = 300,
    CUE_REMARK_RUN_OUT = 301,
    CUE_REMARK_SESSION = 302,
    CUE_UNDEFINED = 303
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

        /* The numeric value
         */
        uint32_t numeric_value;

        /* The string value
         */
	struct cue_string_value
	{
		/* The string data
		 */
	        const char *data;

		/* The string length
		 */
		size_t length;

	} string_value;


};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE libodraw_cue_scanner_lval;

int libodraw_cue_scanner_parse (void *parser_state);

#endif /* !YY_LIBODRAW_CUE_SCANNER_D_CODE_B_VOUND_GITHUB_LIBEWF_EXP_LIBODRAW_LIBODRAW_CUE_PARSER_H_INCLUDED  */
