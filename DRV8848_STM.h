#ifndef __DRV884_STM_H__
#define __DRV884_STM_H__

#include <mbed.h>
#include "DCMotor.h"

typedef struct DRV8848_Params
{
    PinName ain1;
    PinName ain2;
    PinName pwm1;
    PinName bin1;
    PinName bin2;
    PinName pwm2;
    PinName nfault;
    PinName nsleep;
} DRV8848_Params_t;

typedef enum DRV8848_PwmFreq
{
    PWM_21kHz = 0, 
    PWM_12kHz = 1
} DRV8848_PwmFreq_t;

class PwmOutCore2 : public PwmOut
{
public:
    PwmOutCore2(PinName pin)
        : PwmOut(pin){};
    ~PwmOutCore2(){};
    void setupPwm(DRV8848_PwmFreq_t freq);
    void write_fast(float value);
private:
    uint32_t _channel;
    TIM_HandleTypeDef _TimHandle;
};

class DRV8848
{
  public:
    /**
     * @brief DCMotor implementation.
     * @sa DCMotor
     */
    
    class DRVMotor : public DCMotor
    {
      public:
        DRVMotor(PinName pwm, PinName in1, PinName in2);
        ~DRVMotor();
        void setPwmFreq(int freq);
        void setPwmFreq(DRV8848_PwmFreq_t freq);
        void setDutyCycleLimits(float min, float max);        
        bool getDir();
        float getPower();
        void setDriveMode(bool mode);
        bool getDriveMode();
        void togglePolarity();
        void setPolarity(bool polarity);
        void setPower(float pwr);
        void init(int freq);
        float getDutyCycle();

      private:
        DigitalInOut _in1;
        DigitalInOut _in2;
        PwmOutCore2 _pwm;
        bool _polarity;
        bool _cw;
        float _pwr;
        bool _mode;
        float _min_duty_cycle;
        float _max_duty_cycle;
        float _delta_duty_cycle;
    };

    DRV8848(const DRV8848_Params_t *params);
    void stop();
    void setDriveMode(bool mode);
    void enable(bool en = true);
    int getDriverStatus();

    DRVMotor *getDCMotor(MotNum mot_num);
    DRVMotor *operator[](MotNum mot_num)
    {
        return getDCMotor(mot_num);
    }

  private:
    DigitalOut _nsleep;
    DigitalIn _nfault;
    DRVMotor *_mot[2];
    DRV8848_Params_t _params;
};

#endif /*__DRV884_STM_H__*/