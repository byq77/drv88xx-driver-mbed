#ifndef __DRV884_H__
#define __DRV884_H__

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
        DRVMotor(PinName pwm, PinName in1, PinName in2)
            : _in1(in1, PIN_OUTPUT, PushPullNoPull, 0),
              _in2(in2, PIN_OUTPUT, PushPullNoPull, 0),
              _pwm(pwm),
              _polarity(true), _cw(true), _pwr(0), _mode(0)
        {
            setDutyCycleLimits(0, 1);
        }

        ~DRVMotor(){};

        void setPwmFreq(int freq)
        {
            _pwm.period_us(1000000UL / freq);
            _pwm = 0;
        }

        void setDutyCycleLimits(float min, float max)
        {
            _min_duty_cycle = min;
            _max_duty_cycle = max;
            _delta_duty_cycle = max - min;
        }

        bool getDir()
        {
            return _cw;
        }

        float getPower()
        {
            return _pwr;
        }

        void setDriveMode(bool mode)
        {
            _mode = mode;
        }

        bool getDriveMode()
        {
            return _mode;
        }

        void togglePolarity()
        {
            _polarity = !_polarity;
        }

        void setPolarity(bool polarity)
        {
            _polarity = polarity;
        }

        void setPower(float pwr)
        {
            if (pwr == _pwr)
                return;

            // saturation
            _pwr = (pwr < -1.0f ? -1.0f : (pwr > 1.0f ? 1.0 : pwr));

            // direction
            if (_pwr >= 0.0f)
                _cw = (_polarity ? true : false);
            else if (_pwr < 0 )
                _cw = (_polarity ? false : true);

            // stop condition
            if (_pwr == 0)
            {
                _in1.output();
                _in2.output();
                _in1.mode(PushPullNoPull);
                _in2.mode(PushPullNoPull);
                if (_mode)
                {
                    _in1 = 1;
                    _in2 = 1;
                    _pwm = 1;
                }
                else
                {
                    _in2 = 0;
                    _in1 = 0;
                    _pwm = 0;
                }
                return;
            }
            else if (_pwr < 0)
            {
                _pwr = -_pwr;
            }

            // set output pins
            if (_cw)
            {
                if (_mode)
                {
                    _in1.output();
                    _in1.mode(PushPullNoPull);
                    _in1.write(1);
                    _in2.input();
                    _in2.mode(PullNone);
                    _pwr = 1 - _pwr;
                }
                else
                {
                    _in2.output();
                    _in2.mode(PushPullNoPull);
                    _in2.write(0);
                    _in1.input();
                    _in1.mode(PullNone);
                }
            }
            else
            {
                if (_mode)
                {
                    _in2.output();
                    _in2.mode(PushPullNoPull);
                    _in2.write(1);
                    _in1.input();
                    _in1.mode(PullNone);
                    _pwr = 1 - _pwr;
                }
                else
                {
                    _in1.output();
                    _in1.mode(PushPullNoPull);
                    _in1.write(0);
                    _in2.input();
                    _in2.mode(PullNone);
                }
            }
            // set pwm
            _pwm.write(_min_duty_cycle + _pwr * _delta_duty_cycle);
        }

        void init(int freq)
        {
            setPwmFreq(freq);
        }

        float getDutyCycle()
        {
            return _pwm.read();
        }

      private:
        DigitalInOut _in1;
        DigitalInOut _in2;
        PwmOut _pwm;
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

#endif /*__DRV884_H__*/