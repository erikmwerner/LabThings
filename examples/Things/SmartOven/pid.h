#include <PID_v1.h>

double measTemp = 0;
double pidOutput = 0;
double setTemp = 115;
int pidWindowSize = 5000; // 10 s
uint32_t pidWindowStartTime;
uint32_t pidSampleTime = 500; // 500 ms

double Kp = 8000, Ki = 100, Kd = 50;
PID myPID(&measTemp, &pidOutput, &setTemp, Kp, Ki, Kd, P_ON_M, DIRECT);
