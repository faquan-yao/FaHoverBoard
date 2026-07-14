#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "kalman.h"

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from thei
 * driver(s).
 * TODO: The following matrices refer to the configuration on an internal test
 * board at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};

static int32_t q30_scale2(int32_t x)
{
    return (int32_t)((int64_t)x * 2);
}

/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
static unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

static unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

static int run_self_test(void)
{
    int result;
    long gyro[3], accel[3];

    result = mpu_run_self_test(gyro, accel);
    if (result == 0x3) {
        /* Test passed. We can trust the gyro data here, so let's push it down
         * to the DMP.
         */
        float sens;
        unsigned short accel_sens;
        mpu_get_gyro_sens(&sens);
        gyro[0] = (long)(gyro[0] * sens);
        gyro[1] = (long)(gyro[1] * sens);
        gyro[2] = (long)(gyro[2] * sens);
        dmp_set_gyro_bias(gyro);
        mpu_get_accel_sens(&accel_sens);
        accel[0] *= accel_sens;
        accel[1] *= accel_sens;
        accel[2] *= accel_sens;
        dmp_set_accel_bias(accel);
    } else {
        return -1;
    }

    return 0;
}

int MPU6050_DMP_init(void)
{
    int ret;
    struct int_param_s int_param;
    //mpu_init
    ret = mpu_init(&int_param);
    if(ret != 0)
    {
        return ERROR_MPU_INIT;
    }
    //设置传感器
    ret = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    if(ret != 0)
    {
        return ERROR_SET_SENSOR;
    }
    //设置fifo
    ret = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    if(ret != 0)
    {
        return ERROR_CONFIG_FIFO;
    }
    //设置采样率
    ret = mpu_set_sample_rate(DEFAULT_MPU_HZ);
    if(ret != 0)
    {
        return ERROR_SET_RATE;
    }
    //加载DMP固件
    ret = dmp_load_motion_driver_firmware();
    if(ret != 0)
    {
        return ERROR_LOAD_MOTION_DRIVER;
    }
    //设置陀螺仪方向
    ret = dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
    if(ret != 0)
    {
        return ERROR_SET_ORIENTATION;
    }
    //设置DMP功能
    ret = dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
            DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL |
            DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL);
    if(ret != 0)
    {
        return ERROR_ENABLE_FEATURE;
    }
    //设置输出速率
    ret = dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    if(ret != 0)
    {
        return ERROR_SET_FIFO_RATE;
    }
    //自检
//    ret = run_self_test();
//    if(ret != 0)
//    {
//        return ERROR_SELF_TEST;
//    }
    //使能DMP
    ret = mpu_set_dmp_state(1);
    if(ret != 0)
    {
        return ERROR_DMP_STATE;
    }

    return 0;
}

/* Returns pitch/roll/yaw in Q30 radians after Kalman filtering. */
int MPU6050_DMP_Get_Date(int32_t *pitch, int32_t *roll, int32_t *yaw) {
    int32_t q0 = Q30, q1 = 0, q2 = 0, q3 = 0;
    short gyro[3];
    short accel[3];
    long quat[4];
    unsigned long timestamp;
    short sensors;
    unsigned char more;
    int32_t sinp;
    int32_t roll_y;
    int32_t roll_x;
    int32_t yaw_y;
    int32_t yaw_x;
    int32_t p;
    int32_t r;
    int32_t y;

    if (dmp_read_fifo(gyro, accel, quat, &timestamp, &sensors, &more)) {
        return -1;
    }

    if (sensors & INV_WXYZ_QUAT) {
        q0 = (int32_t)quat[0];
        q1 = (int32_t)quat[1];
        q2 = (int32_t)quat[2];
        q3 = (int32_t)quat[3];

        sinp = -q30_scale2(q30_mul(q1, q3)) + q30_scale2(q30_mul(q0, q2));
        p = q30_asin(sinp);

        roll_y = q30_scale2(q30_mul(q2, q3)) + q30_scale2(q30_mul(q0, q1));
        roll_x = Q30 - q30_scale2(q30_mul(q1, q1)) - q30_scale2(q30_mul(q2, q2));
        r = q30_atan2(roll_y, roll_x);

        yaw_y = q30_scale2(q30_mul(q0, q3)) + q30_scale2(q30_mul(q1, q2));
        yaw_x = q30_mul(q0, q0) + q30_mul(q1, q1) - q30_mul(q2, q2) - q30_mul(q3, q3);
        y = q30_atan2(yaw_y, yaw_x);

        *pitch = kalman_update(&pitch_KF, p);
        *roll = kalman_update(&roll_KF, r);
        *yaw = kalman_update(&yaw_KF, y);
    }

    return 0;
}
