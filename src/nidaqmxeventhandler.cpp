#include "nidaqmxeventhandler.h"

// Hi there! I know this flag is really stupid, we shouldn't have made it
// global, but honestly we tried a bunch of stuff and couldn't make this part of
// the eventHandler class and didn't want to mess around changing NIDAQ
// functions too much, so we just made this global variable, and now
// EveryNCallbacks is always called before the meter stops recording, so you are
// guaranteed to have data. If you know of a better way to do this, contact
// us.
bool stopFlag = false;

NIDAQmxEventHandler::NIDAQmxEventHandler(void){};

NIDAQmxEventHandler::NIDAQmxEventHandler(std::string logFilePath) {
  NIDAQmxEventHandler::logfilePath = logFilePath;
}

void NIDAQmxEventHandler::startHandler() {
  std::cout << "me start\n
  int32 error = 0;
  taskHandle = 0;
  char errBuff[2048] = {'\0'};

  /*********************************************/
  // DAQmx Configure Code
  /*********************************************/
  DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
  DAQmxErrChk(DAQmxCreateAIVoltageChan(
      taskHandle, "cDAQ1Mod1/ai0, cDAQ1Mod7/ai0", "", DAQmx_Val_Cfg_Default,
      -10, 10, DAQmx_Val_Volts, NULL));
  DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 10000.0, DAQmx_Val_Rising,
                                    DAQmx_Val_ContSamps, 16000));

  DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(
      taskHandle, DAQmx_Val_Acquired_Into_Buffer, 40, 0, EveryNCallback, NULL));
  DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

  /*********************************************/
  // DAQmx Start Code
  /*********************************************/
  DAQmxErrChk(DAQmxStartTask(taskHandle));

Error:
  if (DAQmxFailed(error)) {
    std::cout << "error has happened";
  }
}

void NIDAQmxEventHandler::tagHandler() { std::cout << "me tag\n"; }

void NIDAQmxEventHandler::endHandler() {
  std::cout << "me end\n";
  stopFlag = true;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle,
                                 int32 everyNsamplesEventType, uInt32 nSamples,
                                 void *callbackData) {
  int32 error = 0;
  char errBuff[2048] = {'\0'};
  static int totalRead = 0;
  int32 read = 0;
  float64 data[BUFFER_SIZE];

  std::string giantString;
  for (size_t index = 0; index < BUFFER_SIZE; index++) {
    giantString += std::to_string(data[index]) + " ";
  }
  std::cout << giantString << "\n\n\n";

  /*********************************************/
  // DAQmx Read Code
  /*********************************************/
  DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, -1, 0, DAQmx_Val_GroupByChannel,
                                 data, 320, &read, NULL));
  if (read > 0) {
    printf("Acquired %d samples. Total %d\r", (int)read,
           (int)(totalRead += read));
    fflush(stdout);
  }
  if (stopFlag) {
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);
  }

Error:
  if (DAQmxFailed(error)) {
    DAQmxGetExtendedErrorInfo(errBuff, 2048);

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