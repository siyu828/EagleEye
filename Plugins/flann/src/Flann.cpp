#include "Flann.h"
#include "thrust/transform.h"
#include "thrust/transform_reduce.h"
#include "thrust/count.h"
#include <opencv2/core/cuda_stream_accessor.hpp>
#include <EagleLib/rcc/external_includes/cv_cudaarithm.hpp>
#include <EagleLib/rcc/external_includes/cv_imgproc.hpp>
#include <parameters/ParameteredObjectImpl.hpp>
#include "flann.cuh"
#include "RuntimeSourceDependency.h"
SETUP_PROJECT_IMPL


using namespace EagleLib;
using namespace EagleLib::Nodes;
void ForegroundEstimate::Init(bool firstInit)
{
	if (firstInit)
	{
		addInputParameter<cv::cuda::GpuMat>("Input point cloud");
		updateParameter<float>("Radius", 5.0);
	}
	updateParameter<boost::function<void(void)>>("Build index", boost::bind(&ForegroundEstimate::BuildModel, this));
}

void ForegroundEstimate::BuildModel()
{
	MapInput(); 
	if (input.cols && input.rows)
	{
		flann::KDTreeCuda3dIndexParams params;
		params["input_is_gpu_float4"] = true;
		flann::Matrix<float> input_ = flann::Matrix<float>((float*)input.data, input.rows, 3, input.step);
		nnIndex.reset(new flann::GpuIndex<flann::L2<float>>(input_, params));
		nnIndex->buildIndex();
	}
}
bool ForegroundEstimate::MapInput(cv::cuda::GpuMat img)
{
	auto pInput = getParameter<cv::cuda::GpuMat>(0)->Data();
	if (!pInput)
	{
		if (img.empty())
			return false;
		pInput = &img;
	}
	if (pInput->depth() != CV_32F)
	{
		NODE_LOG(error) << "Input must be floating point";
		return false;
	}
	if (pInput->channels() != 4)
	{
		if (pInput->channels() != 3)
		{
			NODE_LOG(error) << "Input needs to either be 3 channel XYZ or 4 channels XYZ with unused 4th channel";
			return false;
		}
		// Image is 3 channel, padding magic time
	}
	// Input is 4 channels, awesome no copying needed
	if (!pInput->isContinuous())
	{
		auto buffer = inputBuffer.getFront();
	}
	// Input is 4 channel and continuous... woot
	input = pInput->reshape(1, pInput->rows*pInput->cols);  
	// Input is now a tensor row major matrix
	//input = flann::Matrix<float>((float*)reshaped.data, reshaped.rows, 3, reshaped.step);
	return true;
}
void ForegroundEstimateCallback(int status, void* userData)
{
	static_cast<EagleLib::Nodes::ForegroundEstimate*>(userData)->updateOutput();
}
void ForegroundEstimate::updateOutput()
{
	auto output = outputBuffer.getBack();
	
	cv::cuda::GpuMat filteredCloud = output->data.first.rowRange(0,	output->data.second.createMatHeader().at<int>(0));
	updateParameter("Resulting point cloud", filteredCloud);
}

cv::cuda::GpuMat ForegroundEstimate::doProcess(cv::cuda::GpuMat& img, cv::cuda::Stream& stream)
{
	if (!MapInput(img))
		return img;
	
	auto idxBuffer_ = idxBuffer.getFront();
	auto distBuffer_ = distBuffer.getFront();
	
	idxBuffer_->data.create(1, input.rows*input.cols, CV_32S);
	distBuffer_->data.create(1, input.rows*input.cols, CV_32F);

	flann::Matrix<int> d_idx((int*)idxBuffer_->data.data, input.rows*input.cols, 1, sizeof(int));
	flann::Matrix<float> d_dist((float*)distBuffer_->data.data, input.rows*input.cols, 1, sizeof(int)); 

	flann::SearchParams searchParams;
	searchParams.matrices_in_gpu_ram = true;
	 

	if (nnIndex) 
	{
		auto size = sizeBuffer.getFront();
		flann::Matrix<float> input_ = flann::Matrix<float>((float*)input.data, input.rows, 3, input.step);
		nnIndex->radiusSearch(input_, d_idx, d_dist, *getParameter<float>(1)->Data(), searchParams, cv::cuda::StreamAccessor::getStream(stream));
		auto output = outputBuffer.getFront();
		filterPointCloud(input, output->data.first, idxBuffer_->data, size->data, -1, stream);
		size->data.download(output->data.second); 
		stream.enqueueHostCallback(ForegroundEstimateCallback, this);
		updateParameter("Neighbor index", idxBuffer_->data);
		updateParameter("Neighbor dist", distBuffer_->data);
		//updateParameter("Resulting point cloud", output->data.first);
		//updateParameter("Resulting point cloud size", result);
	}
	else
	{
		NODE_LOG(info) << "Background model not build yet";
	}
	return img;
}

RUNTIME_COMPILER_SOURCEDEPENDENCY_FILE("flann_knl", ".cu")
NODE_DEFAULT_CONSTRUCTOR_IMPL(ForegroundEstimate, PtCloud, Extractor);

