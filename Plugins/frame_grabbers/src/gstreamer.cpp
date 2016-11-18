#include "gstreamer.h"
#include "precompiled.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
using namespace EagleLib;
using namespace EagleLib::Nodes;

frame_grabber_gstreamer::~frame_grabber_gstreamer()
{
    StopThreads();
}
int frame_grabber_gstreamer::CanLoadDocument(const std::string& document)
{
    boost::filesystem::path path(document);
    // oooor a gstreamer pipeline.... 
    std::string appsink = "appsink";
    if(document.find(appsink) != std::string::npos)
        return 9;
    if(boost::filesystem::exists(path))
        return 2;
    LOG(trace) << "Document is not a regular file";
    return 0;
}
std::vector<std::string> frame_grabber_gstreamer::ListLoadableDocuments()
{
    std::vector<std::string> output;
    if(boost::filesystem::exists("file_history.json"))
    {
        boost::property_tree::ptree file_history;
        boost::property_tree::json_parser::read_json("file_history.json", file_history);
        auto files = file_history.get_child_optional("files");
        if(files)
        {
            for(auto itr = files->begin(); itr != files->end(); ++itr)
            {
                auto path = itr->second.get<std::string>("path", "");
                if(path.size())
                    output.push_back(path);
            }
        }
    }
    return output;
}


frame_grabber_gstreamer::frame_grabber_gstreamer():
    frame_grabber_cv()
{
    /*if (!gst_is_initialized())
    {
        char** argv;
        argv = new char*{ "-vvv" };
        int argc = 1;
        gst_init(&argc, &argv);
    }*/
}

bool frame_grabber_gstreamer::LoadFile(const std::string& file_path_)
{
    std::string file_path = file_path_;
    /*if(boost::filesystem::is_regular_file(file_path_))
    {
        file_path = "filesrc location=" + file_path_ + " ! decodebin ! appsink";
    }else
    {
        file_path = file_path_;
    }*/

    if(frame_grabber_cv::h_LoadFile(file_path))
    {
        cv::Mat test;
        if(h_cam)
        {
            if (!h_cam->read(test))
            {
                return false;
            }
        }
        
        h_cam->set(cv::CAP_PROP_POS_FRAMES, 0);
        boost::property_tree::ptree file_history;
        if(boost::filesystem::exists("file_history.json"))
        {
            boost::property_tree::json_parser::read_json("file_history.json", file_history);
        }else
        {
        
        }
        boost::property_tree::ptree child;
        child.put("path", file_path);
        if(!file_history.get_child_optional("files"))
            file_history.add_child("files", boost::property_tree::ptree());
        for(auto& paths : file_history.get_child("files"))
        {
            if(child.get_child("path").get_value<std::string>() == file_path)
            {
                return true;
            }
        }
        file_history.get_child("files").push_back(std::make_pair("", child));
        boost::property_tree::json_parser::write_json("file_history.json", file_history);
        return true;
    }
    return false;
}

rcc::shared_ptr<ICoordinateManager> frame_grabber_gstreamer::GetCoordinateManager()
{
    return coordinate_manager;
}

TS<SyncedMemory> frame_grabber_gstreamer::GetNextFrameImpl(cv::cuda::Stream& stream)
{
    if (d_cam)
    {

    }
    if (h_cam)
    {
        cv::Mat h_mat;
        if (h_cam->read(h_mat))
        {
            if (!h_mat.empty())
            {
                cv::cuda::GpuMat d_mat;
                d_mat.upload(h_mat, stream);
                double pos = h_cam->get(cv::CAP_PROP_POS_AVI_RATIO);
                if(pos == 1.0)
                {
                    sig_eof();
                }
                return TS<SyncedMemory>(h_cam->get(cv::CAP_PROP_POS_MSEC), (long long)h_cam->get(cv::CAP_PROP_POS_FRAMES), h_mat, d_mat);
            }
        }else
        {
            double pos = h_cam->get(cv::CAP_PROP_POS_AVI_RATIO);
            if(pos == 1.0)
            {
                sig_eof();
                if(loop)
                {
                    h_cam->set(cv::CAP_PROP_POS_FRAMES, 0);
                }
            }
        }
    }
    return TS<SyncedMemory>();
}

MO_REGISTER_CLASS(frame_grabber_gstreamer);
