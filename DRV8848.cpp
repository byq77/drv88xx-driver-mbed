#include "DRV8848.h"

#if !USE_STM_DRV8848_DRIVER

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

DRV8848::DRVMotor * DRV8848::getDCMotor(MotNum mot_num)
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