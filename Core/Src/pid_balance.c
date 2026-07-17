/*
 * pid_balance.c
 *
 *  Created on: Dec 23, 2025
 *      Author: yaofa
 */
#include "pid_balance.h"
#include <math.h>

/*
 * Angle PD: u = Kp*err - Kd*gyro
 * Prefer gyro damping over d(error)/dt — much less rock near upright.
 */
PID_TypeDef BalancePID = {
    .Kp = 2.2f,
    .Ki = 0.0f,
    .Kd = 0.12f,    /* on gyro °/s, not on angle difference */
    .MaxOut = 45.0f,
    .MinOut = -45.0f,
    .MaxI = 3.0f,
    .SetVal = 0.0f,
    .NowVal = 0.0f,
    .Err = 0.0f,
    .LastErr = 0.0f,
    .PreErr = 0.0f,
    .Pout = 0.0f,
    .Iout = 0.0f,
    .Dout = 0.0f,
    .Out = 0.0f
};

/* Slow outer trim only — do not fight the angle loop */
PID_TypeDef SpeedPID = {
    .Kp = 0.008f,
    .Ki = 0.0f,
    .Kd = 0.0f,
    .MaxOut = 1.5f,
    .MinOut = -1.5f,
    .MaxI = 1.0f,
    .SetVal = 0.0f,
    .NowVal = 0.0f,
    .Err = 0.0f,
    .LastErr = 0.0f,
    .PreErr = 0.0f,
    .Pout = 0.0f,
    .Iout = 0.0f,
    .Dout = 0.0f,
    .Out = 0.0f
};

void PID_Init(void)
{
    BalancePID.Err = 0.0f;
    BalancePID.LastErr = 0.0f;
    BalancePID.PreErr = 0.0f;
    BalancePID.Iout = 0.0f;
    BalancePID.Out = 0.0f;

    SpeedPID.Err = 0.0f;
    SpeedPID.LastErr = 0.0f;
    SpeedPID.PreErr = 0.0f;
    SpeedPID.Iout = 0.0f;
    SpeedPID.Out = 0.0f;
}

float PID_Calc(PID_TypeDef *pid)
{
    pid->Err = pid->SetVal - pid->NowVal;

    pid->Pout = pid->Kp * pid->Err;

    pid->Iout += pid->Ki * pid->Err;
    if (pid->Iout > pid->MaxI) {
        pid->Iout = pid->MaxI;
    }
    if (pid->Iout < -pid->MaxI) {
        pid->Iout = -pid->MaxI;
    }

    pid->Dout = pid->Kd * (pid->Err - pid->LastErr);

    pid->Out = pid->Pout + pid->Iout + pid->Dout;
    if (pid->Out > pid->MaxOut) {
        pid->Out = pid->MaxOut;
    }
    if (pid->Out < pid->MinOut) {
        pid->Out = pid->MinOut;
    }

    pid->PreErr = pid->LastErr;
    pid->LastErr = pid->Err;

    return pid->Out;
}

float PID_BalanceGyro(PID_TypeDef *pid, float gyro_dps)
{
    pid->Err = pid->SetVal - pid->NowVal;

    pid->Pout = pid->Kp * pid->Err;
    /* Ignore tiny rate noise after bias removal */
    if (fabsf(gyro_dps) < 1.0f) {
        gyro_dps = 0.0f;
    }
    /* Oppose angular rate; GYRO_D_SIGN flips if mount/axis inverted */
    pid->Dout = -GYRO_D_SIGN * pid->Kd * gyro_dps;

    pid->Out = pid->Pout + pid->Dout;
    if (pid->Out > pid->MaxOut) {
        pid->Out = pid->MaxOut;
    }
    if (pid->Out < pid->MinOut) {
        pid->Out = pid->MinOut;
    }

    pid->LastErr = pid->Err;
    return pid->Out;
}
