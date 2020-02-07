#include "nidaqmxeventhandler.h"
#include <iostream>

NIDAQmxEventHandler::NIDAQmxEventHandler(void){};
NIDAQmxEventHandler::~NIDAQmxEventHandler(void){};

NIDAQmxEventHandler::NIDAQmxEventHandler(std::string logFilePath) {
  NIDAQmxEventHandler::logfilePath = logFilePath;
}

void NIDAQmxEventHandler::startHandler() {
  std::cout << "Event Handler: Starting...\n";
  int32 error = 0;
  taskHandle = 0;
  char errBuff[ERRBUFF] = {'\0'};

  /*********************************************/
  // DAQmx Configure Code
  /*********************************************/
  DAQmxErrChk(DAQmxCreateTask("", &taskHandle));

  DAQmxErrChk(DAQmxCreateAIVoltageChan(
      taskHandle, "cDAQ1Mod1/ai0,cDAQ1Mod1/ai5", "", DAQmx_Val_Cfg_Default,
      -10.0, 10.0, DAQmx_Val_Volts, NULL));

  DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, NULL, 10000.0, DAQmx_Val_Rising,
                                    DAQmx_Val_ContSamps, 1000));

  DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(
      taskHandle, DAQmx_Val_Acquired_Into_Buffer, NUM_SAMPLES, 0,
      EveryNCallback, NULL));

  DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

  /*********************************************/
  // DAQmx Start Code
  /*********************************************/
  DAQmxErrChk(DAQmxStartTask(taskHandle));

Error:
  if (DAQmxFailed(error)) {
    DAQmxGetExtendedErrorInfo(errBuff, ERRBUFF);
    printf("DAQmx Error: %s\n", errBuff);
  }
}

void NIDAQmxEventHandler::tagHandler() {
  std::cout << "Event Handler: Tag" << std::endl;
}

void NIDAQmxEventHandler::endHandler() {
  std::cout << "Event Handler: Ending Session..." << std::endl;
  DAQmxStopTask(taskHandle);
  DAQmxClearTask(taskHandle);
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle,
                                 int32 everyNsamplesEventType, uInt32 nSamples,
                                 void *callbackData) {
  int32 error = 0;
  char errBuff[2048] = {'\0'};
  static int totalRead = 0;
  int32 samplesRead = 0;
  float64 data[BUFFER_SIZE];
  std::string giantString;

  /*********************************************/
  // DAQmx Read Code
  /*********************************************/
  DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, -1, 0, DAQmx_Val_GroupByChannel,
                                 data, BUFFER_SIZE, &samplesRead, NULL));
  if (samplesRead > 0) {
    printf("Acquired %d samples. Total %d\r", (int)samplesRead,
           (int)(totalRead += samplesRead));
    fflush(stdout);
  }

  for (size_t index = 0; index < BUFFER_SIZE; index++) {
    giantString += std::to_string(data[index]) + " ";
  }
  //std::cout << giantString << "\n\n\n";

Error:
  if (DAQmxFailed(error)) {
    // Get and print error information
    DAQmxGetExtendedErrorInfo(errBuff, ERRBUFF);

    /*********************************************/
    // DAQmx Stop Code
    /*********************************************/
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);
    printf("DAQmx Error: %s\n", errBuff);
  }
  return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status,
                               void *callbackData) {
  int32 error = 0;
  char errBuff[2048] = {'\0'};

  // Check to see if an error stopped the task.
  DAQmxErrChk(status);

Error:
  if (DAQmxFailed(error)) {
    DAQmxGetExtendedErrorInfo(errBuff, 2048);
    DAQmxClearTask(taskHandle);
    printf("DAQmx Error: %s\n", errBuff);
  }
  return 0;
}