
#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <string>
#include <iostream>
#include <NIDAQmx.h>
#include <stdio.h>




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

#define DAQmxErrChk(functionCall)          \
  if (DAQmxFailed(error = (functionCall))) \
    goto Error;                            \
  else

int NIMeasure(void);
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle,
                                 int32 everyNsamplesEventType, uInt32 nSamples,
                                 void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status,
                               void *callbackData);

class eventHandler {
    public:
        eventHandler(void);
        eventHandler(std::string logFilePath);
        void startHandler();
        void tagHandler();
        void endHandler();    
    private:
        TaskHandle taskHandle;
        std::string logfilePath;
};
#endif