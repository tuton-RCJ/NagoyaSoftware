#include "MPU6050.h"

MPU6050::MPU6050(TwoWire *wire)
{
    mpu = Adafruit_MPU6050();
    mpu.begin(MPU_addr, wire);
    init();


}

void MPU6050::init()
{
    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    mpu.setMotionDetectionThreshold(1);
    mpu.setMotionDetectionDuration(20);
    mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
    mpu.setInterruptPinPolarity(true);
    mpu.setMotionInterrupt(true);
}

void MPU6050::read()
{
    if(mpu.getMotionInterruptStatus())
    {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        AcX = a.acceleration.x;
        AcY = a.acceleration.y;
        AcZ = a.acceleration.z;
        Tmp = temp.temperature;
        GyX = g.gyro.x;
        GyY = g.gyro.y;
        GyZ = g.gyro.z;
    }
}

void MPU6050::getValues(int *values)
{
    values[0] = AcX;
    values[1] = AcY;
    values[2] = AcZ;
    values[3] = Tmp;
    values[4] = GyX;
    values[5] = GyY;
    values[6] = GyZ;
}
