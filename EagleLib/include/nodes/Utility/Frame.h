#pragma once
#include "nodes/Node.h"

namespace EagleLib
{
    class FrameRate: public Node
    {
        boost::posix_time::ptime prevTime;
    public:
        FrameRate();
        virtual void Init(bool firstInit);
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream stream = cv::cuda::Stream::Null());
    };
	
	class FrameLimiter : public Node
	{
	public:
		FrameLimiter();
		virtual void Init(bool firstInit);
		virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream stream = cv::cuda::Stream::Null());
	};
}