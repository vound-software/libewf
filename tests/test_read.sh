#!/bin/bash
#
# Expert Witness Compression Format (EWF) library read testing script
#
# Copyright (c) 2006-2010, Joachim Metz <jbmetz@users.sourceforge.net>
#
# Refer to AUTHORS for acknowledgements.
#
# This software is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this software.  If not, see <http://www.gnu.org/licenses/>.
#

EXIT_SUCCESS=0;
EXIT_FAILURE=1;
EXIT_IGNORE=77;

INPUT="input";
TMP="tmp";

CMP="cmp";
LS="ls";
TR="tr";
SED="sed";
SORT="sort";
UNIQ="uniq";
WC="wc";

function test_read
{ 
	echo "Testing read of input:" $*;

	./${EWF_TEST_READ} $*;

	RESULT=$?;

	echo "";

	return ${RESULT};
}

EWF_TEST_READ="ewf_test_read";

if ! test -x ${EWF_TEST_READ};
then
	EWF_TEST_READ="ewf_test_read.exe";
fi

if ! test -x ${EWF_TEST_READ};
then
	echo "Missing executable: ${EWF_TEST_READ}";

	exit ${EXIT_FAILURE};
fi

if ! test -d ${INPUT};
then
	echo "No ${INPUT} directory found, to test read create ${INPUT} directory and place EWF test files in directory.";
	echo "Use unique filename bases per set of EWF image file(s)."

	exit ${EXIT_IGNORE};
fi

RESULT=`${LS} ${INPUT} | ${TR} ' ' '\n' | ${SED} 's/[.][^.]*$//' | ${SORT} | ${UNIQ} | ${WC} -l`;

if test ${RESULT} -eq 0;
then
	echo "No files found in ${INPUT} directory, to test read place EWF test files in directory.";
	echo "Use unique filename bases per set of EWF image file(s)."

	exit ${EXIT_IGNORE};
fi

for BASENAME in `${LS} ${INPUT} | ${TR} ' ' '\n' | ${SED} 's/[.][^.]*$//' | ${SORT} | ${UNIQ}`;
do
	if ! test_read `${LS} ${INPUT}/${BASENAME}.*`;
	then
		exit ${EXIT_FAILURE};
	fi
done

exit ${EXIT_SUCCESS};

