/*********************************************************************
*
* ANSI C Example program:
*    ContAcq-IntClk.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a continuous amount of
*    data using the DAQ device's internal clock.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage range.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*    3. Set the rate of the acquisition. Also set the Samples per
*       Channel control. This will determine how many samples are
*       read at a time. This also determines how many points are
*       plotted on the graph each time.
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Set the rate for the sample clock. Additionally, define the
*       sample mode to be continuous.
*    4. Call the Start function to start the acquistion.
*    5. Read the data in the EveryNCallback function until the stop
*       button is pressed or an error occurs.
*    6. Call the Clear Task function to clear the task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include "NIDAQmx.h"

#include <windows.h>
#include "nidaqmx_server.h"

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);


float64 nidaq_diff_volts[NUM_NIDQ_CHANNELS];
float64 nidaq_chan_resistor = 0.003 ;
#if 0
float64 nidaq_chan_volts[] = 
{
	11.95, 11.95, 11.93, 11.95, 11.95, 11.95, 11.95, 11.95, /* 0-7:    NIDAQ1 MOD1 channel 0-7 */
	11.95, 11.94, 11.94, 11.94, 11.95, 11.95, 11.95, 11.93, /* 8-15:   NIDAQ1 MOD1 channel 16-23 */
	 3.31,  3.31, 11.98,  3.31,  3.31,  5.04,  5.04,  5.04,  /* 16-23:  NIDAQ1 MOD8 channel 0-7 */
	 3.31 /* 16-23:  NIDAQ1 MOD8 channel 16 this one needs to check! */
};
/* supermicro pin-map 0-7 & 16 power supply; 17-23 xeon phi */
float64 nidaq_chan_volts[] = 
{
	12.03, 12.03, 12.03, 12.03, 12.03, 12.03, 12.03, 12.03, /* 0-7:    NIDAQ1 MOD1 channel 0-7 */
	12.03, 11.99, 11.99, 11.99, 11.99, 11.99, 11.99, 11.99 /* 8-15:   NIDAQ1 MOD1 channel 16-23 */
};
#endif
/* all xeon phi: 0-7 & 16 pcie; 17-23 8pin & 6pin */
float64 nidaq_chan_volts[] = 
{
	12.00, 12.00, 12.00, 12.00, 12.00, 3.3, 3.3, 3.3, /* 0-7:    NIDAQ1 MOD1 channel 0-7 */
	3.3, 11.99, 11.99, 11.99, 11.99, 11.99, 11.99, 11.99 /* 8-15:   NIDAQ1 MOD1 channel 16-23 */
};

int start_nidaq_meter(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	/* Modified by HC: 03292014 */
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"cDAQ1Mod1/ai0:7, cDAQ1Mod1/ai16:23","", DAQmx_Val_Diff,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,16000));

	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandle,DAQmx_Val_Acquired_Into_Buffer,20,0,EveryNCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	while(running)
	{
		if(DAQmxFailed(error)) {
		    DAQmxGetExtendedErrorInfo(errBuff,2048);
		    printf("DAQmx Error: %s\n",errBuff);
			running = 0;
		}
		Sleep(100);
	}
	//printf("Acquiring samples continuously. Press Enter to interrupt\n");
	//getchar();

Error:
/*
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
*/
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
/*
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
*/
	return 0;
}

static void nidaq_diff_volt_to_power(float64 *ChanReading, int num_chan)
{
	int i;
	unsigned int milli_powers[32];

	memcpy(nidaq_diff_volts, ChanReading, sizeof(float64)*num_chan);
#if 1
	for (i=0; i<num_chan; i++)
		milli_powers[i] = (unsigned int)(((nidaq_diff_volts[i]/nidaq_chan_resistor)*(nidaq_chan_volts[i] - nidaq_diff_volts[i]))*1000);
#endif
#if 0
	for (i=0; i<num_chan; i++) {
		milli_powers[i] = (unsigned int)(((nidaq_diff_volts[i]/nidaq_chan_resistor)*(nidaq_chan_volts[i] - nidaq_diff_volts[i]))*1000);
		if (i > 8) printf("%ld ", milli_powers[i]);
	}
	printf("\n");
#endif

	WaitForSingleObject( hMutex, INFINITE );
	for (i=0; i<num_chan; i++)
		nidaq_milli_powers[i] = milli_powers[i];
	ReleaseMutex( hMutex );
	//(voltage / maps[channel_id].resistor) * (maps[channel_id].ps_voltage - voltage);
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
	static int  totalRead=0;
	int32       read=0;
	float64     data[25000];

    int			numChan=NUM_ACTIVE_NIDQ_CHANNELS;
	int			i, j;
	float64		ChanBuff[NUM_ACTIVE_NIDQ_CHANNELS];
	FILETIME now;
    double currentTime;

	//GetSystemTimeAsFileTime(&now);

// Convert to UTC time in seconds since 0:00 Jan 1 1970
	//currentTime = (now.dwHighDateTime * 4294962796e-7 - 11644473600.0) + now.dwLowDateTime * 1e-7;

	//printf("%10.10f ", currentTime);

	//printf( "%ul ", ((FILETIME *)&currentlongtime));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,-1,0,DAQmx_Val_GroupByChannel,data,320,&read,NULL));
	if( read>0 ) {

		for(i=0; i<numChan; i++) {
			ChanBuff[i] = 0.0;
			for(j=0; j<read; j++)
				ChanBuff[i] += data[j+i*read];
			ChanBuff[i] /= read;
			if(ChanBuff[i]<0)
				ChanBuff[i] = 0.0;
			//printf("%f ", ChanBuff[i]);
		}
		nidaq_diff_volt_to_power(ChanBuff, NUM_ACTIVE_NIDQ_CHANNELS);
#if 0
		printf("%f %d samples/channel\n", ChanBuff[0], read);
		printf("Acquired %d samples. Total %d. numChan %d\r",read,totalRead+=read,numChan);
		fflush(stdout);
#endif
	}

	//nidaq_diff_volt_to_power(ChanBuff, NUM_NIDQ_CHANNELS);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}
