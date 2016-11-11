#include "TParameterResource.hpp"
#include <MetaObject/Parameters/IO/SerializationFunctionRegistry.hpp>
#include <cereal/archives/json.hpp>
using namespace vclick;

void TParameterResource<EagleLib::SyncedMemory>::handleParamUpdate(mo::Context* ctx, mo::IParameter* param)
{
    std::stringstream* new_ss = new std::stringstream();
    auto func = mo::SerializationFunctionRegistry::Instance()->
        GetJsonSerializationFunction(param->GetTypeInfo());
    dynamic_cast<mo::ITypedParameter<EagleLib::SyncedMemory>*>(param)->GetDataPtr()->Synchronize();
    if (func)
    {
        cereal::JSONOutputArchive ar(*new_ss);
        func(param, ar);
        std::stringstream* old_ss;
        {
            std::lock_guard<std::mutex> lock(mtx);
            old_ss = ss;
            ss = new_ss;
        }
        delete old_ss;
    }
}