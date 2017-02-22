#include "ImageDisplay.h"

#include <MetaObject/Thread/InterThread.hpp>
#include <EagleLib/utilities/CudaCallbacks.hpp>
#include <EagleLib/utilities/UiCallbackHandlers.h>
#include <MetaObject/Logging/Profiling.hpp>



using namespace EagleLib;
using namespace EagleLib::Nodes;

bool QtImageDisplay::ProcessImpl()
{
    cv::Mat mat;
    bool stream_desynced = false;
    long long ts = -1;
    bool overlay = overlay_timestamp;
    if(image && !image->empty())
    {
        if(image->GetSyncState() == EagleLib::SyncedMemory::DEVICE_UPDATED)
        {
            stream_desynced = true;
        }
        mat = image->GetMat(Stream());
        ts = image_param.GetTimestamp();
    }
    if(cpu_mat)
    {
        mat = *cpu_mat;
    }

    std::string name = GetTreeName();
    if(!mat.empty())
    {
        size_t gui_thread_id = mo::ThreadRegistry::Instance()->GetThread(mo::ThreadRegistry::GUI);

        EagleLib::cuda::enqueue_callback_async(
            [mat, name, overlay, ts, this]()->void
        {
            PROFILE_RANGE(imshow);
            cv::Mat draw_img = mat;
            if(overlay)
            {
                draw_img = mat.clone();
                std::stringstream ss;
                ss << "Timestamp: " << ts;
                cv::putText(mat, ss.str(), cv::Point(20,40), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(0,255,0));
            }
            GetDataStream()->GetWindowCallbackManager()->imshow(name, draw_img);
        }, gui_thread_id, Stream());

        return true;
    }
    return false;
}


bool KeyPointDisplay::ProcessImpl()
{
    return true;
}

bool FlowVectorDisplay::ProcessImpl()
{
    return true;
}

bool HistogramDisplay::ProcessImpl()
{
    return true;
}

bool DetectionDisplay::ProcessImpl()
{
    return true;
}

bool OGLImageDisplay::ProcessImpl()
{
    //cv::namedWindow(name, cv::WINDOW_OPENGL);
    std::string name = GetTreeName();
    //_pixel_buffer.copyFrom(image->GetGpuMat(Stream()), Stream());
    size_t gui_thread_id = mo::ThreadRegistry::Instance()->GetThread(mo::ThreadRegistry::GUI);
    cv::cuda::GpuMat gpumat = image->GetGpuMat(Stream());
    EagleLib::cuda::enqueue_callback_async(
                [name, this, gpumat]()->void
    {
        PROFILE_RANGE(imshow);
        //cv::imshow(name, _pixel_buffer);
        //GetDataStream()->GetWindowCallbackManager()->imshowb(name, _pixel_buffer, cv::WINDOW_OPENGL);
        GetDataStream()->GetWindowCallbackManager()->imshowd(name, gpumat, cv::WINDOW_OPENGL);
    }, gui_thread_id, Stream());
    return true;
}


MO_REGISTER_CLASS(QtImageDisplay)
MO_REGISTER_CLASS(KeyPointDisplay)
MO_REGISTER_CLASS(FlowVectorDisplay)
MO_REGISTER_CLASS(HistogramDisplay)
MO_REGISTER_CLASS(DetectionDisplay)
MO_REGISTER_CLASS(OGLImageDisplay)
