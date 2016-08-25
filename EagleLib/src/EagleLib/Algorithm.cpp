#include "EagleLib/Algorithm.h"
#include "EagleLib/Detail/AlgorithmImpl.hpp"
#include <MetaObject/Parameters/InputParameter.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/rolling_window.hpp>

using namespace mo;
using namespace EagleLib;


Algorithm::Algorithm()
{
    _pimpl = new impl();
    _enabled = true;
}

Algorithm::~Algorithm()
{
    delete _pimpl;
    _pimpl = nullptr;
}

double Algorithm::GetAverageProcessingTime() const
{
    return 0.0;
}

void Algorithm::SetEnabled(bool value)
{
    _enabled = value;
}

bool Algorithm::IsEnabled() const
{
    return _enabled;
}

void Algorithm::Process()
{
    if(_enabled == false)
        return;
    if(!CheckInputs())
    {
        return;
    }
    ProcessImpl();

    if(_pimpl->sync_input == nullptr && _pimpl->ts != -1)
        ++_pimpl->ts;
    _pimpl->last_ts = _pimpl->ts;
}

bool Algorithm::CheckInputs()
{
    auto inputs = this->GetInputs();
    if(inputs.size() == 0)
        return true;
    if(_pimpl->sync_input != nullptr)
    {
        _pimpl->ts = _pimpl->sync_input->GetTimestamp();
        LOG(trace) << "Timestamp updated to " << _pimpl->ts;
    }
    if(_pimpl->ts == -1)
    {
        for(auto input : inputs)
        {
            long long ts = input->GetTimestamp();
            if(ts != -1)
            {
                if(_pimpl->ts == -1)
                    _pimpl->ts = ts;
                else
                    _pimpl->ts = std::min(_pimpl->ts, ts);
            }
        }
        LOG(trace) << "Timestamp updated to " << _pimpl->ts;
    }
    if(_pimpl->ts == _pimpl->last_ts && _pimpl->last_ts != -1)
        return false;
    for(auto input : inputs)
    {
        if(!input->GetInput(_pimpl->ts))
        {
            LOG(trace) << input->GetTreeName() << " failed to get input at timestamp: " << _pimpl->ts;
            return false;
        }
    }
    return true;
}

void Algorithm::Clock(int line_number)
{
    
}

long long Algorithm::GetTimestamp() 
{
    return _pimpl->ts;
}

void Algorithm::SetSyncInput(const std::string& name)
{
    _pimpl->sync_input = GetInput(name);
}