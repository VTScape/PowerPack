#include "eventhandler.h"

eventHandler::eventHandler(void){};

eventHandler::eventHandler(std::string logFilePath) {
  eventHandler::logfilePath = logFilePath;
}

void eventHandler::startHandler() {
  std::cout << "me start\n";
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
      taskHandle, DAQmx_Val_Acquired_Into_Buffer, 20, 0, EveryNCallback, NULL));
  DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

  /*********************************************/
  // DAQmx Start Code
  /*********************************************/
  DAQmxErrChk(DAQmxStartTask(taskHandle));

Error:
    if (DAQmxFailed(error)){
      std::cout << "error has happened";
    }
}

void eventHandler::tagHandler() { std::cout << "me tag\n"; }

void eventHandler::endHandler() {
  std::cout << "me end\n";
  DAQmxStopTask(taskHandle);
  DAQmxClearTask(taskHandle);
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle,
                                 int32 everyNsamplesEventType, uInt32 nSamples,
                                 void *callbackData) {
  int32 error = 0;
  char errBuff[2048] = {'\0'};
  static int totalRead = 0;
  int32 read = 0;
  float64 data[10000];

  std::string giantString;
  for(size_t index = 0; index < 10000; index++) {
    giantString += std::to_string(data[index]) + " ";
  }
  std::cout << giantString << "\n";

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