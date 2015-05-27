#pragma once
#include "nodes/Node.h"
#include <opencv2/videoio.hpp>
#include <CudaUtils.hpp>
namespace EagleLib
{
    class Camera: public Node
    {
        virtual bool changeStream(const std::string& gstreamParams);
        virtual bool changeStream(int device);
        cv::VideoCapture cam;
        cv::cuda::HostMem hostBuf;
        //BufferPool<cv::cuda::GpuMat, EventPolicy, LockedPolicy> imageBuffer;

//        boost::thread acquisitionThread;
//        void acquisitionLoop();
    public:
        Camera();
        ~Camera();
        virtual void Init(bool firstInit);
        virtual void Serialize(ISimpleSerializer *pSerializer);
        virtual bool SkipEmpty() const;
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream& stream);
    };
    class GStreamerCamera: public Node
    {
        cv::VideoCapture cam;
        cv::cuda::HostMem hostBuf;
        enum SourceType
        {
            v4l2src = 0
        };
        enum VideoType
        {
            h264 = 0
        };
        void setString();
    public:
        GStreamerCamera();
        virtual void Init(bool firstInit);
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream &stream);
        virtual bool SkipEmpty() const;
    };
}
