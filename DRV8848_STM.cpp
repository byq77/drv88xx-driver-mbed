#include "DRV8848_STM.h"
#include "pwmout_device.h"

#if defined(TARGET_CORE2) && USE_STM_DRV8848_DRIVER

typedef struct ARR_and_PSC
{
    uint32_t PSC;
    uint32_t ARR;
} ARR_and_PSC_t;

static const ARR_and_PSC_t PWM_SETUP_CORE2[]=
{
    {(uint32_t)7, (uint32_t)1000},
    {(uint32_t)13, (uint32_t)1000}
};

void PwmOutCore2::setupPwm(DRV8848_PwmFreq_t freq)
{
    int i = 0;
    core_util_critical_section_enter();
    lock_deep_sleep();
    _TimHandle.Instance = (TIM_TypeDef *)(_pwm.pwm);
    __HAL_TIM_DISABLE(&this->_TimHandle);

    /*  Parse the pwm / apb mapping table to find the right entry */
    while (pwm_apb_map_table[i].pwm != _pwm.pwm)
    {
        i++;
    }
    
    if (pwm_apb_map_table[i].pwm == 0)
    {
        error("Unknown PWM instance");
    }
    
    if (pwm_apb_map_table[i].pwmoutApb == PWMOUT_ON_APB1)
    {
        this->_TimHandle.Init.Prescaler = (((PWM_SETUP_CORE2[freq].PSC + 1)>>1) - 1);
    }
    else
    {
#if !defined(PWMOUT_APB2_NOT_SUPPORTED)
        this->_TimHandle.Init.Prescaler = (uint32_t)PWM_SETUP_CORE2[freq].PSC;
#endif
    }
    this->_TimHandle.Init.Period = PWM_SETUP_CORE2[freq].ARR;
    this->_TimHandle.Init.ClockDivision = 0;
    this->_TimHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    
    if (HAL_TIM_PWM_Init(&this->_TimHandle) != HAL_OK) {
        error("Cannot initialize PWM\n");
    }

    // Save for future use
    _pwm.prescaler = this->_TimHandle.Init.Prescaler+1;
    _pwm.period = this->_TimHandle.Init.Period;
    _pwm.pulse = (uint32_t)0;

    switch (_pwm.channel)
    {
    case 1:
        this->_channel = TIM_CHANNEL_1;
        break;
    case 2:
        this->_channel = TIM_CHANNEL_2;
        break;
    case 3:
        this->_channel = TIM_CHANNEL_3;
        break;
    case 4:
        this->_channel = TIM_CHANNEL_4;
        break;
    default:
        return;
    }

    write_fast(0);

    __HAL_TIM_ENABLE(&this->_TimHandle);

    core_util_critical_section_exit();
}

void PwmOutCore2::write_fast(float value)
{
    _pwm.pulse = (uint32_t)((float)_pwm.period * value + 0.5);
     __HAL_TIM_SET_COMPARE(&this->_TimHandle, this->_channel, _pwm.pulse);    
#if !defined(PWMOUT_INVERTED_NOT_SUPPORTED)
    if (_pwm.inverted) {
        HAL_TIMEx_PWMN_Start(&this->_TimHandle, this->_channel);
    } else
#endif
    {
        HAL_TIM_PWM_Start(&this->_TimHandle, this->_channel);
    }
}

DRV8848::DRVMotor::DRVMotor(PinName pwm, PinName in1, PinName in2)
    : _in1(in1, PIN_OUTPUT, PushPullNoPull, 0),
      _in2(in2, PIN_OUTPUT, PushPullNoPull, 0),
      _pwm(pwm),
      _polarity(true), _cw(true), _pwr(0), _mode(0)
{
    setDutyCycleLimits(0, 1);
}

DRV8848::DRVMotor::~DRVMotor(){};

void DRV8848::DRVMotor::setPwmFreq(int freq)
{
    _pwm.period_us(1000000UL / freq);
    _pwm.write(0.0f);
}

void DRV8848::DRVMotor::setPwmFreq(DRV8848_PwmFreq_t freq)
{
    _pwm.setupPwm(freq);
    _pwm.write_fast(0.0f);
}

void DRV8848::DRVMotor::setDutyCycleLimits(float min, float max)
{
    _min_duty_cycle = min;
    _max_duty_cycle = max;
    _delta_duty_cycle = max - min;
}

inline bool DRV8848::DRVMotor::getDir()
{
    return _cw;
}

inline float DRV8848::DRVMotor::getPower()
{
    return _pwr;
}

inline void DRV8848::DRVMotor::setDriveMode(bool mode)
{
    _mode = mode;
}

inline bool DRV8848::DRVMotor::getDriveMode()
{
    return _mode;
}

void DRV8848::DRVMotor::init(int freq)
{
    (void)freq;
    setPwmFreq(PWM_21kHz);
}

inline void DRV8848::DRVMotor::togglePolarity()
{
    _polarity = !_polarity;
}

inline void DRV8848::DRVMotor::setPolarity(bool polarity)
{
    _polarity = polarity;
}

void DRV8848::DRVMotor::setPower(float pwr)
{
    if (pwr == _pwr)
        return;

    // saturation
    _pwr = (pwr < -1.0f ? -1.0f : (pwr > 1.0f ? 1.0 : pwr));

    // direction
    if (_pwr >= 0.0f)
        _cw = (_polarity ? true : false);
    else if (_pwr < 0)
        _cw = (_polarity ? false : true);

    // stop condition
    if (_pwr == 0)
    {
        core_util_critical_section_enter();
        _in1.output();
        _in2.output();
        _in1.mode(PushPullNoPull);
        _in2.mode(PushPullNoPull);
        if (_mode)
        {
            _in1 = 1;
            _in2 = 1;
            _pwm.write_fast(1.0f);
        }
        else
        {
            _in2 = 0;
            _in1 = 0;
            _pwm.write_fast(0.0f);
        }
        core_util_critical_section_exit();
        return;
    }
    else if (_pwr < 0)
    {
        _pwr = -_pwr;
    }

    // set output pins
    core_util_critical_section_enter();
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
    _pwm.write_fast(_pwr);
    core_util_critical_section_exit();
}

inline float DRV8848::DRVMotor::getDutyCycle()
{
    return _pwm.read();
}

DRV8848::DRV8848(const DRV8848_Params_t * params)
: _nsleep(params->nsleep,0)
, _nfault(params->nfault,PullUp)
, _params(*params)
{
    for(int i=0;i<2;i++)
        _mot[i] = NULL;
}

int DRV8848::getDriverStatus()
{
    return _nfault.read();
}

void DRV8848::enable(bool en)
{
    _nsleep.write((int)en);
}

void DRV8848::stop()
{
    for(int i=0;i<2;i++)
    {
        if(_mot[i] != NULL)
            _mot[i]->setPower(0);
    }
}

void DRV8848::setDriveMode(bool mode)
{
    for(int i=0;i<2;i++)
    {
        if(_mot[i] != NULL)
            _mot[i]->setDriveMode(mode);
    }
}

DRV8848::DRVMotor *DRV8848::getDCMotor(MotNum mot_num)
{
    if(_mot[mot_num] == NULL)
    {
        switch(mot_num)
        {
            case MOT1:
                if(_params.ain1 != NC && _params.ain2 != NC && _params.pwm1 != NC)
                    _mot[(int)mot_num] = new DRVMotor(_params.pwm1, _params.ain1, _params.ain2);
                else
                    return NULL;
                break;
            case MOT2:
                if(_params.bin1 != NC && _params.bin2 != NC && _params.pwm2 != NC)
                    _mot[(int)mot_num] = new DRVMotor(_params.pwm2, _params.bin1, _params.bin2);
                else
                    return NULL;
                break;
            default:
                return NULL;
        }
    }
    return _mot[(int)mot_num];
}

#endif
