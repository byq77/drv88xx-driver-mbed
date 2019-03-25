#ifndef __DC_MOTOR_H__
#define __DC_MOTOR_H__

#include <mbed.h>

/**
 * @brief Motor selector.
 */
typedef enum
{
    MOT1 = 0,
    MOTA = 0,
    MOT2 = 1,
    MOTB = 1,
}MotNum;

/**
 * @brief DCMotor interface.
 */
class DCMotor
{
  public:
    
    virtual ~DCMotor(){};
  
    /**
     * @brief Initialise motor.
     * @param pwm_frequency frequency on pwm pin
     */
    virtual void init(float freq)=0;

    /**
     * @brief Set motor power.
     * @param power corresponds to motor speed where 0 is no rotation and 1 or -1 are max speed
     * @param mode drive mode 
     */
    virtual void setPower(float pwr)=0;

    /**
     * @brief Set pwm frequency
     * @param freq frequency in Hz
     */
    virtual void setPwmFreq(int freq)=0;

    /**
     * @brief Get duty cycle on pwm pin.
     * @return duty cycle
     */
    virtual float getDutyCycle()=0;

    /**
     * @brief Get current power setting.
     * @return power
     */
    virtual float getPower()=0;

    /**
     * @brief Get motor direction.
     * @return true if cw, false if ccw 
     */
    virtual bool getDir()=0;

    /**
     * @brief Set motor polarity.
     * @param polarity new polarity 
     */
    virtual void setPolarity(bool polarity)=0;

    /**
     * @brief Toggle motor polarity.
     */
    virtual void togglePolarity()=0;

    /**
     * @brief Set Drive Mode.
     * @param mode true - Fast Decay Mode, false - Slow Decay Mode 
     */
    virtual void setDriveMode(bool mode)=0;

    /**
     * @brief Get Drive Mode.
     * @return mode true - Fast Decay Mode, false - Slow Decay Mode 
     */
    virtual bool getDriveMode()=0;

    /**
     * @brief Set duty cycle limits.
     * @param min - lower limit
     * @param max - upper limit
     */
    virtual void setDutyCycleLimits(float min, float max)=0;
};

#endif /*__DC_MOTOR_H__*/