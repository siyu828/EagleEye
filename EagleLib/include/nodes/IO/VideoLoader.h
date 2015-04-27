#include <nodes/Node.h>

#include <opencv2/cudacodec.hpp>
#include <opencv2/videoio.hpp>


namespace EagleLib
{

    class CV_EXPORTS VideoLoader : public EventLoopNode
    {
        bool load;
    public:
        VideoLoader();
        ~VideoLoader();
        void Init(bool firstInit);
        void loadFile();
        void restartVideo();
        virtual void Serialize(ISimpleSerializer *pSerializer);
        virtual bool SkipEmpty() const;
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat& img, cv::cuda::Stream stream = cv::cuda::Stream::Null());
        cv::Ptr<cv::cudacodec::VideoReader> d_videoReader;
        cv::Ptr<cv::VideoCapture>           h_videoReader;

    };
}