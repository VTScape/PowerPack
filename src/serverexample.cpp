#include "functionapi.h"
#include "nimeasure.h"

TaskHandle start() {
  std::cout << "me start\n";
  int32 error = 0;
  TaskHandle taskHandle = 0;
  char errBuff[2048] = {'\0'};

  /*********************************************/
  // DAQmx Configure Code
  /*********************************************/
  DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
  DAQmxErrChk(DAQmxCreateAIVoltageChan(
      taskHandle, "cDAQ3Mod1/ai0, cDAQ3Mod2/ai0", "", DAQmx_Val_Cfg_Default,
      -10.0, 10.0, DAQmx_Val_Volts, NULL));
  DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 10000.0, DAQmx_Val_Rising,
                                    DAQmx_Val_ContSamps, 16000));

  DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(
      taskHandle, DAQmx_Val_Acquired_Into_Buffer, 20, 0, EveryNCallback, NULL));
  DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

  /*********************************************/
  // DAQmx Start Code
  /*********************************************/
  DAQmxErrChk(DAQmxStartTask(taskHandle));

  printf("Acquiring samples continuously. Press Enter to interrupt\n");
  return taskHandle;
Error:
  std::cout << "error has happened";
  return taskHandle;
}

void end(TaskHandle taskHandle) {
  std::cout << "me end\n";
  DAQmxStopTask(taskHandle);
  DAQmxClearTask(taskHandle);
}

void tag() { std::cout << "me tag\n"; }

int main(int argc, char** argv) {
  std::string configFile(argv[1]);
  socketServer server =
      initializeMeterServer(readServerConfig(configFile), start, end, tag);
  server.listenForClient();
  return 0;
}
