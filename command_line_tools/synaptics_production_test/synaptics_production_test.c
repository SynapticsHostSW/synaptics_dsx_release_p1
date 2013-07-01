/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Copyright (c) 2011, 2012 Synaptics Incorporated. All rights reserved.
 *
 * The information in this file is confidential under the terms
 * of a non-disclosure agreement with Synaptics and is provided
 * AS IS without warranties or guarantees of any kind.
 *
 * The information in this file shall remain the exclusive property
 * of Synaptics and may be the subject of Synaptics patents, in
 * whole or part. Synaptics intellectual property rights in the
 * information in this file are not expressly or implicitly licensed
 * or otherwise transferred to you as a result of such information
 * being made available to you.
 *
 * File: read_report.c
 *
 * Description: command line production test implimentation using command
 * line args. This file should not be OS dependant and should build and
 * run under any Linux based OS that utilizes the Synaptice rmi driver
 * built into the kernel.
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include "test_limit.h"

#include <libxls/xls.h>


#define VERSION "1.3"

#define DEFAULT_SENSOR "/sys/class/input/input0"

#define MAX_STRING_LEN 256
#define MAX_INT_LEN 33
#define PATIENCE 3

#define DATA_FILENAME "f54/report_data"
#define DO_PREPARATION_FILENAME "f54/do_preparation"
#define REPORT_TYPE_FILENAME "f54/report_type"
#define GET_REPORT_FILENAME "f54/get_report"
#define REPORT_SIZE_FILENAME "f54/report_size"
#define STATUS_FILENAME "f54/status"
#define RESET_FILENAME "reset"
#define MAPPED_TX_SIZE_FILENAME "f54/num_of_mapped_tx"
#define MAPPED_RX_SIZE_FILENAME "f54/num_of_mapped_rx"

char mySensor[MAX_STRING_LEN];
char testLimitXls[MAX_STRING_LEN];
unsigned char *buffer;
unsigned short numberOfMappedTx;
unsigned short numberOfMappedRx;
unsigned short showAll = 0;
int skip_preparation = 0;
int argsXls = -1;
int debug_externel_limits = 0;

enum test_results {
	TEST_NONE,
	TEST_PASS,
	TEST_FAILED,
};

static void usage(char *name)
{
	printf("Version %s\n", VERSION);
	printf("Usage: %s {report_type} [-d {sysfs_entry}] [-s]\n", name);
	printf("\t[-d {sysfs_entry}] - Path to sysfs entry of sensor\n");
	printf("\t[-s] - Skip preparation procedures\n");
	printf("\t[-i] - Show all test results\n");
	printf("\t[-r] - Run 4 tests\n");
	printf("\t[-x {file_name.xls}] - Load test limits from .xls file\n");

	return;
}

void printReportTypeInfo(int reportType)
{
	printf("\n%s\n", ReportTypeString[reportType]);
}

int LoadTestLimits(char *argv)
{
	xlsWorkBook* pWB;
	xlsWorkSheet* pWS;
	int i, j;
	int r, c;
	struct st_row_data* row;

	pWB=xls_open(argv,"UTF-8");

	if (pWB!=NULL)
	{
	
	for (i=0;i<pWB->sheets.count;i++) {
		if (debug_externel_limits)
			printf("Sheet N%i (%s) pos %i\n",
			i,
			pWB->sheets.sheet[i].name,
			pWB->sheets.sheet[i].filepos);
        
	    for (j=0; j<4; j++) {
			if (!strcmp(pWB->sheets.sheet[i].name, testLimitsSheets[j])) {

				if (debug_externel_limits)
					printf("Start to parse limits%s(%d)\n",
						testLimitsSheets[j], j);

				pWS=xls_getWorkSheet(pWB,i);
			        	xls_parseWorkSheet(pWS);

				switch (j) {
				case 0: //"FullRawCapacitance"
					for (r = 0; r < numberOfMappedTx; r++) {
						for (c = 0; c < numberOfMappedRx; c++) {

						  row=&pWS->rows.row[r+1];

						 if (debug_externel_limits) {
							 printf("FullRawCapacitance L %f\n",
							 	(&row->cells.cell[2*c+1])->d);
							 printf("FullRawCapacitance H %f\n",
							 	(&row->cells.cell[2*c+2])->d);
						}
						FullRaeCapLowerLimit[r][c] = (&row->cells.cell[2*c+1])->d;
						FullRaeCapUpperLimit[r][c] = (&row->cells.cell[2*c+2])->d;
						}
					}
					break;
				case 1: //"TxToTx"
					r = 1;
					for (c = 0; c < numberOfMappedTx; c++) {
						row=&pWS->rows.row[r];

						if (debug_externel_limits)
							 printf("TxToTx %f\n", (&row->cells.cell[c+1])->d);
						TxTxReportLimit[c] =  (&row->cells.cell[c+1])->d;
					}
					break;
				case 2:	//"RxToRx"
					//TODO
					for (r = 0; r < numberOfMappedRx; r++) {
						for (c = 0; c < numberOfMappedRx; c++) {
						
						  row=&pWS->rows.row[r+1];

							if (debug_externel_limits) {
							 printf("RxShortLowerLimit L %f\n",
						 		(&row->cells.cell[2*c + 1])->d);
							 printf("RxShortUpperLimit H %f\n",
						 		(&row->cells.cell[2*c + 2])->d);
							}
						RxShortLowerLimit[r][c] = (&row->cells.cell[2*c+1])->d;
						RxShortUpperLimit[r][c] = (&row->cells.cell[2*c+2])->d;
						}
					}
					break;

				case 3:	//"HighResistance" 
					r = 1;
					for (c = 0; c < 3 ; c++) {
						row=&pWS->rows.row[r];

						if (debug_externel_limits) {
						 printf("HighResistance L %f\n", (&row->cells.cell[2*c+1])->d);
						 printf("HighResistance H %f\n", (&row->cells.cell[2*c+2])->d);
						}

						HighResistanceLowerLimit[c] = (float)(&row->cells.cell[2*c+1])->d;
						HighResistanceUpperLimit[c] = (float)(&row->cells.cell[2*c+2])->d;
					}
					break;	
				default:
					printf("[%d] Invalid value!\n", i);
					break;
				}
		           }
	    	}
	}
	} else {
		printf("File %s not exist!\n", argv);
		exit(EIO);
	}
    return 0;
}

static void ReadBinData(char *fname, unsigned char *buf, int len)
{
	int numBytesRead;
	FILE *fp;

	fp = fopen(fname, "rb");
	if (!fp) {
		printf("ERROR: failed to open %s for reading data\n", fname);
		exit(EIO);
	}

	numBytesRead = fread(buf, 1, len, fp);

	if (numBytesRead != len) {
		printf("ERROR: failed to read all data from bin file\n");
		fclose(fp);
		exit(EIO);
	}

	fclose(fp);

	return;
}

static void WriteValueToFp(FILE *fp, unsigned int value)
{
	int numBytesWritten;
	char buf[MAX_INT_LEN];

	snprintf(buf, MAX_INT_LEN, "%u", value);

	fseek(fp, 0, 0);

	numBytesWritten = fwrite(buf, 1, strlen(buf) + 1, fp);
	if (numBytesWritten != ((int)(strlen(buf) + 1))) {
		printf("ERROR: failed to write value to file pointer\n");
		fclose(fp);
		exit(EIO);
	}

	return;
}

static void WriteValueToSysfsFile(char *fname, unsigned int value)
{
	FILE *fp;

	fp = fopen(fname, "w");
	if (!fp) {
		printf("ERROR: failed to open %s for writing value\n", fname);
		exit(EIO);
	}

	WriteValueToFp(fp, value);

	fclose(fp);

	return;
}

static void ReadValueFromFp(FILE *fp, unsigned int *value)
{
	int retVal;
	char buf[MAX_INT_LEN];

	fseek(fp, 0, 0);

	retVal = fread(buf, 1, sizeof(buf), fp);
	if (retVal == -1) {
		printf("ERROR: failed to read value from file pointer\n");
		exit(EIO);
	}

	*value = strtoul(buf, NULL, 0);

	return;
}

static void ReadValueFromSysfsFile(char *fname, unsigned int *value)
{
	FILE *fp;

	fp = fopen(fname, "r");
	if (!fp) {
		printf("ERROR: failed to open %s for reading value\n", fname);
		exit(EIO);
	}

	ReadValueFromFp(fp, value);

	fclose(fp);

	return;
}

static void ReadBlockData(char *buf, int len)
{
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, DATA_FILENAME);

	ReadBinData(tmpfname, (unsigned char *)buf, len);

	return;
}

static void DoPreparation(int value)
{
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, DO_PREPARATION_FILENAME);

	WriteValueToSysfsFile(tmpfname, value);

	return;
}

static void SetReportType(int value)
{
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, REPORT_TYPE_FILENAME);

	WriteValueToSysfsFile(tmpfname, value);

	return;
}

static void GetReport(int value)
{
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, GET_REPORT_FILENAME);

	WriteValueToSysfsFile(tmpfname, value);

	return;
}

static int ReadReportSize(void)
{
	unsigned int reportSize;
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, REPORT_SIZE_FILENAME);

	ReadValueFromSysfsFile(tmpfname, &reportSize);

	return reportSize;
}

static int ReadMappedTxSize(void)
{
	unsigned int reportSize;
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, MAPPED_TX_SIZE_FILENAME);

	ReadValueFromSysfsFile(tmpfname, &reportSize);

	return reportSize;
}

static int ReadMappedRxSize(void)
{
	unsigned int reportSize;
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, MAPPED_RX_SIZE_FILENAME);

	ReadValueFromSysfsFile(tmpfname, &reportSize);

	return reportSize;
}

static int GetStatus(void)
{
	unsigned int status;
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, STATUS_FILENAME);

	ReadValueFromSysfsFile(tmpfname, &status);

	return status;
}

static void DoReset(int value)
{
	char tmpfname[MAX_STRING_LEN];

	snprintf(tmpfname, MAX_STRING_LEN, "%s/%s", mySensor, RESET_FILENAME);

	WriteValueToSysfsFile(tmpfname, value);

	return;
}

static _Bool CheckTestLimit(void)
{
	if ((numberOfMappedTx > CFG_F54_TXCOUNT) || (numberOfMappedRx > CFG_F54_RXCOUNT)) {
		printf("[Warning] TX/RX mapped count read from device T%d/R%d greater than settings T%d/R%d\n",
					numberOfMappedTx,
					numberOfMappedRx,
					CFG_F54_TXCOUNT,
					CFG_F54_RXCOUNT);
		return 0;
	}
	return 1;
}

static enum test_results FullRawCapacitanceTest()
{
 	int i, j, k=0;
	char Result;
	float *DataArray;
	unsigned short numberOfColums = numberOfMappedTx;
	unsigned short numberOfRows = numberOfMappedRx;
	_Bool isValidTestLimit = CheckTestLimit();
	short temp;
	enum test_results ret = TEST_PASS;

	DataArray = (float*)malloc(sizeof(float)*numberOfMappedTx*numberOfMappedRx);

	for (i = 0; i < numberOfColums; i++) {
	   	for (j = 0; j < numberOfRows; j++) {
			temp = buffer[k] | (buffer[k+1] << 8);
			DataArray[i*numberOfRows+j] = ((float)temp)/1000;
			k = k + 2;
		}  	  
   	}

	// Check against test limits
	if (showAll)
		printf("\nThe test result:\n");
	for (i = 0; i < numberOfColums; i++) {
	   	for (j = 0; j < numberOfRows; j++) {
			if (isValidTestLimit) {
			    	if ((DataArray[i*numberOfRows+j] < FullRaeCapUpperLimit[i][j]) &&
					(DataArray[i*numberOfRows+j] > FullRaeCapLowerLimit[i][j]))
		   			Result = 'P'; /*Pass*/
				else {
					ret = TEST_FAILED;
					Result = 'F'; /*Fail*/;
				}
			} else 
				Result = ' ';
			if (showAll || Result == 'F')
				printf("\t[%2d][%2d] (%6.3f) %c\n", i, j, DataArray[i*numberOfRows+j], Result);
	  	}
	   	if (showAll)
			printf("\n");
	}

	free(DataArray);
	return ret;
}

static enum test_results TxToTxShortTest()
{
 	int i, j, index;
	int numberOfBytes = (numberOfMappedTx + 7) / 8;
	char val;
	char Result;
	_Bool isValidTestLimit;
	enum test_results ret = TEST_PASS;

	isValidTestLimit = CheckTestLimit();

	for (i = 0; i < numberOfBytes; i++) {
		for (j = 0; j < 8; j++) {
			index =  i*8+j;
			if (index >= numberOfMappedTx)
				break;
			val = (buffer[i] & (1 << j)) >>j;
			if (isValidTestLimit) {
				if (val == TxTxReportLimit[index])
					Result = 'P';
				else {
					ret = TEST_FAILED;
					Result = 'F';
				}
			} else
				Result = ' ';

			if (showAll || Result == 'F')
				printf("\tTx [%2d] %d (%d) %c\n",
					index,
					val,
					TxTxReportLimit[index],
					Result);
		}
   	}
	return ret;
}

static enum test_results RxToRxShortTest(int rxTimes)
{
	int i, j, k=0;
	char Result;
	float *DataArray;
	unsigned short numberOfColums = numberOfMappedTx;
	unsigned short numberOfRows = numberOfMappedRx;
	//_Bool isValidTestLimit = CheckTestLimit();
	short temp;
	enum test_results ret = TEST_PASS;
	float upper_limit;
	float lower_limit;

	DataArray = (float*)malloc(sizeof(float)*numberOfMappedTx*numberOfMappedRx);

	for (i = 0; i < numberOfColums; i++) {
		for (j = 0; j < numberOfRows; j++) {
			temp = buffer[k] | (buffer[k+1] << 8);
			DataArray[i*numberOfRows+j] = ((float)temp)/1000;
			k = k + 2;
		}
	}

	// Check against test limits
	if (showAll)
		printf("\nThe test result:\n");
	for (i = 0; i < numberOfColums; i++) {
	   	for (j = 0; j < numberOfRows; j++) {
			if ((i + (numberOfColums * rxTimes)) >= numberOfRows) {				
				//over bounding
				break;
				} else {
					upper_limit = RxShortUpperLimit[i+ (numberOfColums * rxTimes)][j];
					lower_limit = RxShortLowerLimit[i+ (numberOfColums * rxTimes)][j];
				if ((DataArray[i*numberOfRows + j] < upper_limit) &&
					(DataArray[i*numberOfRows + j] > lower_limit))
		   			Result = 'P'; /*Pass*/
				else {
					ret = TEST_FAILED;
					Result = 'F'; /*Failed*/
				}
				if (showAll || Result == 'F')
					printf("\t[%2d][%2d] (%6.3f) %c\tMaximum (%6.3f) , Minimum(%6.3f)\n", (i+ (numberOfColums * rxTimes)), j, DataArray[i*numberOfRows + j], Result, upper_limit, lower_limit);
			}
	  	}
	   	if (showAll)
			printf("\n");
	}

	free(DataArray);
	return ret;
}

static enum test_results HighResistanceTest()
{
	float HIghResistanceResult[3];
	int i, k=0;
	short temp;
	char Result;
	enum test_results ret = TEST_PASS;
	
	for (i = 0; i <3; i++, k+=2) {
		temp = buffer[k] | (buffer[k+1] << 8);
		HIghResistanceResult[i] = ((float)temp)/1000;

		if ((HIghResistanceResult[i] < HighResistanceUpperLimit[i]) &&
			(HIghResistanceResult[i] > HighResistanceLowerLimit[i]))	
			Result = 'P';
		else {
			Result = 'F';
			ret = TEST_FAILED;
		}
		if (showAll || Result == 'F')
			printf("\t[%d] (%6.3f) %c\n", i, HIghResistanceResult[i], Result);
	}
	return ret;
}

enum test_results runTest(int report_type)
{
	int ii;
	int rx_report = 0;
	int report_size;
	unsigned char patience = PATIENCE;
	unsigned char data_8;
	char *report_data_8;
	short *report_data_16;
	int *report_data_32;
	enum test_results ret = TEST_NONE;

	printReportTypeInfo(report_type);

	if (!skip_preparation) {
		switch (report_type) {
		case F54_16BIT_IMAGE:
		case F54_RAW_16BIT_IMAGE:
		case F54_ABS_CAP:
		case F54_ABS_DELTA:
			break;
		default:
			DoPreparation(1);
			break;
		}
	}
	
setReport:
	SetReportType(report_type);

	GetReport(1);

	do {
		sleep(1);
		if (GetStatus() == 0)
			break;
	} while (--patience > 0);

	report_size = ReadReportSize();
	if (report_size == 0) {
		DoReset(1);
		printf("ERROR: unable to read report\n");
		exit(EINVAL);
	}
	if (showAll)
		printf("report size %d\n", report_size);

	buffer = malloc(report_size);
	if (!buffer) {
		DoReset(1);
		exit(ENOMEM);
	}
	ReadBlockData((char *)&buffer[0], report_size);

	switch (report_type) {
	case F54_8BIT_IMAGE:
		report_data_8 = (char *)buffer;
		for (ii = 0; ii < report_size; ii++) {
			printf("%03d: %d\n", ii, *report_data_8);
			report_data_8++;
		}
		break;
	case F54_FULL_RAW_CAP_RX_COUPLING_COMP:	
		ret = FullRawCapacitanceTest();
		break;
	case F54_TX_TO_TX_SHORT:
		ret = TxToTxShortTest();
		break;
	case F54_RX_TO_RX1:
	case F54_RX_TO_RX2:
	case F54_RX_TO_RX3:
	case F54_RX_TO_RX4:
		if (ret== TEST_FAILED)
			RxToRxShortTest(rx_report);
		else
			ret = RxToRxShortTest(rx_report);
		rx_report++;
		if (numberOfMappedTx * rx_report < numberOfMappedRx) {
			report_type = RxToRxShortReports[rx_report];
			goto setReport;
		}
		break;
	case F54_HIGH_RESISTANCE:
		ret = HighResistanceTest();
		break;
	case F54_16BIT_IMAGE:
	case F54_RAW_16BIT_IMAGE:
	case F54_TRUE_BASELINE:
	case F54_FULL_RAW_CAP_MIN_MAX:
	case F54_FULL_RAW_CAP:
	case F54_SENSOR_SPEED:
	case F54_ADC_RANGE:
		report_data_16 = (short *)buffer;
		for (ii = 0; ii < report_size; ii += 2) {
			printf("%03d: %d\n", ii / 2, *report_data_16);
			report_data_16++;
		}
		break;
	case F54_ABS_CAP:
	case F54_ABS_DELTA:
		report_data_32 = (int *)buffer;
		for (ii = 0; ii < report_size; ii += 4) {
			printf("%03d: %d\n", ii / 4, *report_data_32);
			report_data_32++;
		}
		break;
	case F54_ABS_ADC:
		report_data_16 = (short *)buffer;
		for (ii = 0; ii < report_size; ii += 2) {
			data_8 = (unsigned char)*report_data_16;
			printf("%03d: %d\n", ii / 2, data_8);
			report_data_16++;
		}
		break;
	default:
		for (ii = 0; ii < report_size; ii++)
			printf("%03d: 0x%02x\n", ii, buffer[ii]);
		break;
	}

	free(buffer);
	DoReset(1);

	return ret;
}

int main(int argc, char* argv[])
{
	int i;
	int this_arg = 1;
	int report_type;
	struct stat st;
	_Bool singleTest = 0;
	enum test_results ret;
	enum test_results ret_all = TEST_PASS;

	if (argc == 1) {
		usage(argv[0]);
		exit(EINVAL);
	}

	while (this_arg < argc) {
		if (!strcmp((const char *)argv[this_arg], "-d")) {
			/* path to sensor sysfs entry */
			this_arg++;

			if (stat(argv[this_arg], &st) == 0) {
				strncpy(mySensor, argv[this_arg], MAX_STRING_LEN);
			} else {
				printf("ERROR: sensor sysfs entry %s not found\n", argv[this_arg]);
				exit(EINVAL);
			}
		} else if (!strcmp((const char *)argv[this_arg], "-s")) {
			skip_preparation = 1;
		} else if (!strcmp((const char *)argv[this_arg], "-i")) {
			showAll = 1;
		} else if (!strcmp((const char *)argv[this_arg], "-r")) {
			printf("Run %d tests\n", sizeof(RunTests)/sizeof(enum report_types));
		} else if (!strcmp((const char *)argv[this_arg], "-x")) {
			/* path to .xls file */
			this_arg++;
			argsXls = this_arg;
			printf("Load test limit file %s \n", argv[argsXls]);
		} else {
			singleTest = 1;
			report_type = strtoul(argv[this_arg], NULL, 0);
			printf("Run single test %d \n", report_type);
		}
		this_arg++;
	}

	if (!strlen(mySensor))
		strncpy(mySensor, DEFAULT_SENSOR, MAX_STRING_LEN);

	if (showAll)
		printf("Sersor entry %s\n", mySensor);

	numberOfMappedTx = ReadMappedTxSize();
	numberOfMappedRx = ReadMappedRxSize();

	if (showAll)
		printf("Tx number %d, Rx number %d\n",
			numberOfMappedTx,
			numberOfMappedRx);
	//check test limit .xls file
	if (argsXls != -1)
		LoadTestLimits(argv[argsXls]);
	else
		printf("Use default test limits.\n");

	if (singleTest) {
		ret = runTest(report_type);
		
		if (ret == TEST_PASS)
			printf("\nTEST RESULT : PASS\n");
		else if (ret == TEST_NONE)
			printf("\nTest Finished.\n");
		else
			printf("\nTEST RESULT : FAILED!!!\n");		
	}
	else {
		for (i = 0; i < sizeof(RunTests)/sizeof(enum report_types) ; i++) {
			ret = runTest(RunTests[i]);
			if (ret == TEST_FAILED)
			{
				ret_all = TEST_FAILED;
				printf("Report type %d Failed\n",RunTests[i]);
			}
				
		}
		if (ret_all == TEST_PASS)
			printf("\nTEST RESULT : PASS\n");
		else if (ret == TEST_NONE)
			printf("\nTest Finished.\n");
		else
			printf("\nTEST RESULT : FAILED!!!\n");
	}
	

	return 0;
}
