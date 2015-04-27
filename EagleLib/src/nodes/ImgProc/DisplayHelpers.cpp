#include "nodes/ImgProc/DisplayHelpers.h"
using namespace EagleLib;
#include <opencv2/cudaarithm.hpp>


#ifdef _MSC_VER

#else
RUNTIME_COMPILER_LINKLIBRARY("-lopencv_cudaarithm")
#endif

void
AutoScale::Init(bool firstInit)
{

}

cv::cuda::GpuMat
AutoScale::doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream stream)
{
    std::vector<cv::cuda::GpuMat> channels;
    cv::cuda::split(img,channels);
    for(int i = 0; i < channels.size(); ++i)
    {
        double minVal, maxVal;
        cv::cuda::minMax(channels[i], &minVal, &maxVal);
        double scaleFactor = 255.0 / (maxVal - minVal);
        channels[i].convertTo(channels[0], CV_8U, scaleFactor, minVal*scaleFactor);
        updateParameter<double>("Min-" + boost::lexical_cast<std::string>(i), minVal, Parameter::State);
        updateParameter<double>("Max-" + boost::lexical_cast<std::string>(i), maxVal, Parameter::State);
    }
    cv::cuda::merge(channels,img);
    return img;
}

void
Colormap::Init(bool firstInit)
{
    resolution = 5000;
    updateParameter("Colormapping scheme", int(0));
    updateParameter("Colormap resolution", &resolution);
}

cv::cuda::GpuMat
Colormap::doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream stream)
{
    if(img.channels() != 1)
    {
        log(Warning, "Non-monochrome image! Has " + boost::lexical_cast<std::string>(img.channels()) + " channels");
        return img;
    }
    if(LUT.size() != resolution)
    {
        double minVal, maxVal;
        cv::cuda::minMax(img, &minVal,&maxVal);
        scale = double(resolution - 1) / (maxVal - minVal);
        shift = minVal * scale;
        updateParameter<double>("Min", minVal, Parameter::State);
        updateParameter<double>("Max", maxVal, Parameter::State);
        updateParameter<double>("Scale", scale, Parameter::State);
        updateParameter<double>("Shift", shift, Parameter::State);
        buildLUT();
    }
    cv::cuda::GpuMat scaledImg;
    img.convertTo(scaledImg, CV_16U, scale,shift);
    cv::Mat h_img;
    scaledImg.download(h_img);
    cv::Mat colorScaledImage(h_img.size(),CV_8UC3);
    cv::Vec3b* putPtr = colorScaledImage.ptr<cv::Vec3b>(0);
    unsigned short* getPtr = h_img.ptr<unsigned short>(0);
    for(int i = 0; i < h_img.rows*h_img.cols; ++i, ++putPtr, ++ getPtr)
    {
        *putPtr = LUT[*getPtr];
    }
    return cv::cuda::GpuMat(colorScaledImage);
}


void
Colormap::buildLUT()
{
    //thrust::host_vector<cv::Vec3b> h_LUT;
    int scalingScheme = getParameter<int>(0)->data;
    switch(scalingScheme)
    {
    case 0:
    default:
        red     = ColorScale(50, 255/25, true);
        green   = ColorScale(50 / 3, 255/25, true);
        blue    = ColorScale(0, 255/25, true);
        break;
    }
    LUT.resize(resolution);
    blue.inverted = true;
    // color scales are defined between 0 and 100
    double step = 100.0 / double(resolution);
    double location = 0.0;
    for(int i = 0; i < resolution; ++i, location += step)
    {
        LUT[i] = cv::Vec3b(blue(location), green(location), red(location));
    }
    //d_LUT = h_LUT;
}



ColorScale::ColorScale(double start_, double slope_, bool symmetric_)
{
    start = start_;
    slope = slope_;
    symmetric = symmetric_;
    flipped = false;
    inverted = false;
}
uchar ColorScale::operator ()(double location)
{
    return getValue(location);
}

uchar ColorScale::getValue(double location_)
{
    double value = 0;
    if (location_ > start)
    {
        value = (location_ - start)*slope;
    }
    else
    {
        value = 0;
    }
    if (value > 255)
    {
        if (symmetric) value = 512 - value;
        else value = 255;
    }
    if (value < 0) value = 0;
    if (inverted) value = 255 - value;
    return (uchar)value;
}
void Normalize::Init(bool firstInit)
{
    updateParameter<double>("Alpha", 0);
    updateParameter<double>("Beta", 1);
}

cv::cuda::GpuMat Normalize::doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream stream)
{
    cv::cuda::normalize(img,img, getParameter<double>(0)->data, getParameter<double>(1)->data, CV_MINMAX, img.type());
    return img;
}

NODE_DEFAULT_CONSTRUCTOR_IMPL(AutoScale);
NODE_DEFAULT_CONSTRUCTOR_IMPL(Colormap);
NODE_DEFAULT_CONSTRUCTOR_IMPL(Normalize);