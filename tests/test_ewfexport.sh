#!/usr/bin/env bash
# Export tool testing script
#
# Version: 20240413

EXIT_SUCCESS=0;
EXIT_FAILURE=1;
EXIT_IGNORE=77;

PROFILES=("ewfexport" "ewfexport_chunk" "ewfexport_multi" "ewfexport_chunk_multi")
OPTIONS_PER_PROFILE=("-j0 -q -texport -u" "-j0 -q -texport -u -x" "-j4 -q -texport -u" "-j4 -q -texport -u -x")
OPTION_SETS="format:encase1 format:encase2 format:encase3 format:encase4 format:encase5 format:encase6 format:encase7 format:encase7-v2 format:ewf format:ewfx format:ftk format:linen5 format:linen6 format:linen7 format:raw format:smart deflate:none deflate:empty-block deflate:fast deflate:best blocksize:16 blocksize:32 blocksize:128 blocksize:256 blocksize:512 blocksize:1024 blocksize:2048 blocksize:4096 blocksize:8192 blocksize:16384 blocksize:32768 hash:sha1 hash:sha256 hash:all";

INPUT_GLOB="*.[Ees]*01";

test_callback()
{
	local TMPDIR=$1;
	local TEST_SET_DIRECTORY=$2;
	local TEST_OUTPUT=$3;
	local TEST_EXECUTABLE=$4;
	local TEST_INPUT=$5;
	shift 5;
	local ARGUMENTS=("$@");

	TEST_EXECUTABLE=$( readlink_f "${TEST_EXECUTABLE}" );
	INPUT_FILE_FULL_PATH=$( readlink_f "${INPUT_FILE}" );

	(cd ${TMPDIR} && run_test_with_input_and_arguments "${TEST_EXECUTABLE}" "${INPUT_FILE_FULL_PATH}" ${ARGUMENTS[@]});
	local RESULT=$?;

	if test -f "${TMPDIR}/export.raw";
	then
		local TEST_LOG="${TEST_OUTPUT}.log";

		if test "${PLATFORM}" = "Darwin";
		then
			(cd ${TMPDIR} && md5 export.* | sort -k 2 > "${TEST_LOG}");
		else
			(cd ${TMPDIR} && md5sum export.* | sort -k 2 > "${TEST_LOG}");
		fi

		local TEST_RESULTS="${TMPDIR}/${TEST_LOG}";
		local STORED_TEST_RESULTS="${TEST_SET_DIRECTORY}/${TEST_LOG}.gz";

		if test -f "${STORED_TEST_RESULTS}";
		then
			# Using zcat here since zdiff has issues on Mac OS X.
			# Note that zcat on Mac OS X requires the input from stdin.
			zcat < "${STORED_TEST_RESULTS}" | diff "${TEST_RESULTS}" -;
			RESULT=$?;
		else
			gzip ${TEST_RESULTS};

			mv "${TEST_RESULTS}.gz" ${TEST_SET_DIRECTORY};
		fi
	else
		run_test_with_input_and_arguments "${VERIFY_TOOL}" ${TMPDIR}/export.* -q > /dev/null;
		RESULT=$?;
	fi
	return ${RESULT};
}

if test -n "${SKIP_TOOLS_TESTS}" || test -n "${SKIP_TOOLS_END_TO_END_TESTS}";
then
	exit ${EXIT_IGNORE};
fi

TEST_EXECUTABLE="../ewftools/ewfexport";

if ! test -x "${TEST_EXECUTABLE}";
then
	TEST_EXECUTABLE="../ewftools/ewfexport.exe";
fi

if ! test -x "${TEST_EXECUTABLE}";
then
	echo "Missing test executable: ${TEST_EXECUTABLE}";

	exit ${EXIT_FAILURE};
fi

VERIFY_TOOL="../ewftools/ewfverify";

if ! test -x "${VERIFY_TOOL}";
then
	VERIFY_TOOL="../ewftools/ewfverify.exe";
fi

if ! test -x "${VERIFY_TOOL}";
then
	echo "Missing executable: ${VERIFY_TOOL}";

	exit ${EXIT_FAILURE};
fi

TEST_DIRECTORY=`dirname $0`;

TEST_RUNNER="${TEST_DIRECTORY}/test_runner.sh";

if ! test -f "${TEST_RUNNER}";
then
	echo "Missing test runner: ${TEST_RUNNER}";

	exit ${EXIT_FAILURE};
fi

source ${TEST_RUNNER};

PLATFORM=`uname -s`;

if test "${PLATFORM}" = "Darwin";
then
	assert_availability_binary md5;
else
	assert_availability_binary md5sum;
fi

if ! test -d "input";
then
	echo "Test input directory not found.";

	exit ${EXIT_IGNORE};
fi
RESULT=`ls input/* | tr ' ' '\n' | wc -l`;

if test ${RESULT} -eq ${EXIT_SUCCESS};
then
	echo "No files or directories found in the test input directory";

	exit ${EXIT_IGNORE};
fi

for PROFILE_INDEX in ${!PROFILES[*]};
do
	TEST_PROFILE=${PROFILES[${PROFILE_INDEX}]};

	TEST_PROFILE_DIRECTORY=$(get_test_profile_directory "input" "${TEST_PROFILE}");

	IGNORE_LIST=$(read_ignore_list "${TEST_PROFILE_DIRECTORY}");

	IFS=" " read -a PROFILE_OPTIONS <<< ${OPTIONS_PER_PROFILE[${PROFILE_INDEX}]};

	RESULT=${EXIT_SUCCESS};

	for TEST_SET_INPUT_DIRECTORY in input/*;
	do
		if ! test -d "${TEST_SET_INPUT_DIRECTORY}";
		then
			continue;
		fi
		TEST_SET=`basename ${TEST_SET_INPUT_DIRECTORY}`;

		if check_for_test_set_in_ignore_list "${TEST_SET}" "${IGNORE_LIST}";
		then
			continue;
		fi
		TEST_SET_DIRECTORY=$(get_test_set_directory "${TEST_PROFILE_DIRECTORY}" "${TEST_SET_INPUT_DIRECTORY}");

		RESULT=${EXIT_SUCCESS};

		if test -f "${TEST_SET_DIRECTORY}/files";
		then
			IFS="" read -a INPUT_FILES <<< $(cat ${TEST_SET_DIRECTORY}/files | sed "s?^?${TEST_SET_INPUT_DIRECTORY}/?");
		else
			IFS="" read -a INPUT_FILES <<< $(ls -1d ${TEST_SET_INPUT_DIRECTORY}/${INPUT_GLOB});
		fi
		for INPUT_FILE in "${INPUT_FILES[@]}";
		do
			TESTED_WITH_OPTIONS=0;

			for OPTION_SET in ${OPTION_SETS[@]};
			do
				TEST_DATA_OPTION_FILE=$(get_test_data_option_file "${TEST_SET_DIRECTORY}" "${INPUT_FILE}" "${OPTION_SET}");

				if test -f ${TEST_DATA_OPTION_FILE};
				then
					TESTED_WITH_OPTIONS=1;

					IFS=" " read -a OPTIONS <<< $(read_test_data_option_file "${TEST_SET_DIRECTORY}" "${INPUT_FILE}" "${OPTION_SET}");

					run_test_on_input_file "${TEST_SET_DIRECTORY}" "ewfexport" "with_callback" "${OPTION_SET}" "${TEST_EXECUTABLE}" "${INPUT_FILE}" "${PROFILE_OPTIONS[@]}" "${OPTIONS[@]}";
					RESULT=$?;

					if test ${RESULT} -ne ${EXIT_SUCCESS};
					then
						break;
					fi
				fi
			done

			if test ${TESTED_WITH_OPTIONS} -eq 0;
			then
				run_test_on_input_file "${TEST_SET_DIRECTORY}" "ewfexport" "with_callback" "" "${TEST_EXECUTABLE}" "${INPUT_FILE}" "${PROFILE_OPTIONS[@]}";
				RESULT=$?;
			fi

			if test ${RESULT} -ne ${EXIT_SUCCESS};
			then
				break;
			fi
		done

		# Ignore failures due to corrupted data.
		if test "${TEST_SET}" = "corrupted";
		then
			RESULT=${EXIT_SUCCESS};
		fi
		if test ${RESULT} -ne ${EXIT_SUCCESS};
		then
			break;
		fi
	done
done

exit ${RESULT};

