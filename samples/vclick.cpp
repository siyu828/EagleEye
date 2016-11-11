//#include "vclick.hpp"

#include "WebSink.hpp"
#include "vclick.hpp"
#include "MetaObject/Detail/IMetaObjectImpl.hpp"
#include "MetaObject/Parameters/IO/SerializationFunctionRegistry.hpp"

#include <cereal/types/vector.hpp>
#include <EagleLib/IO/cvMat.hpp>
#include "EagleLib/IO/memory.hpp"
#include "EagleLib/Nodes/NodeFactory.h"


#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WJavaScriptSlot>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>

#include <boost/functional.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

#include <fstream>

using namespace EagleLib;
using namespace EagleLib::Nodes;
using namespace vclick;
using namespace Wt;

rcc::shared_ptr<WebSink> g_sink;



WApplication* createApplication(const Wt::WEnvironment& env)
{
    return new WebUi(env);
}

void WebUi::StartServer(int argc, char** argv, rcc::shared_ptr<WebSink> sink)
{
    g_sink = sink;
    Wt::WRun(argc, argv, &createApplication);
}

WebUi::WebUi(const Wt::WEnvironment& env):
    WApplication(env),
    onMove(this, "onMove"),
    onResize(this, "onResize"),
    onRotate(this, "onRotate"),
    onKeydown(this, "onKeydown")
{
    setTitle("vclick3d demo");
    
    auto stream = g_sink->GetDataStream();
    auto fg = stream->GetNode("ForegroundEstimate0");
    backgroundStream = new TParameterResource<EagleLib::SyncedMemory>(
        fg->GetParameter("background_model"), "background");

    foregroundStream = new TParameterResource<cv::Mat>(
        g_sink->GetParameter("foreground_points"), "foreground");

    boundingBoxStream = new TParameterResource<std::vector<BoundingBox>>(
        g_sink->GetParameter("bounding_boxes"), "bounding_boxes");

    auto background_link = Wt::WLink(backgroundStream);
    auto foreground_link = Wt::WLink(foregroundStream);
    auto boundingBox_link = Wt::WLink(boundingBoxStream);
    
    {
        std::ofstream ofs("./web/bb.json");
        cereal::JSONOutputArchive ar(ofs);
        auto bb = g_sink->GetParameter<std::vector<BoundingBox>>("bounding_boxes");

        auto func = mo::SerializationFunctionRegistry::Instance()->
            GetJsonSerializationFunction(bb->GetTypeInfo());
        if (func)
        {
            func(bb, ar);
        }
    }

    Wt::WPushButton *activate = new WPushButton("Activate", root());
    activate->clicked().connect(this, &WebUi::handleActivate);
    

    Wt::WPushButton *trigger = new WPushButton("AddTriggerRegion", root());
    trigger->clicked().connect(this, &WebUi::handleAddbb);
    

    Wt::WPushButton *rebuild = new WPushButton("Rebuild Model", root());
    rebuild->clicked().connect(this, &WebUi::handleRebuildModel);
    

    if (!require("three.js"))
    {
        BOOST_LOG_TRIVIAL(info) << "Unable to open three.js";
    }

    if (!require("js/libs/stats.min.js"))
    {
        BOOST_LOG_TRIVIAL(info) << "Unable to open stats.min.js";
    }
    if(!require("js/controls/TrackballControls.js"))
    {
        BOOST_LOG_TRIVIAL(info) << "Unable to open TrackballControls.js";
    }
    if(!require("js/controls/OrbitControls.js"))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load OrbitControls.js";
    }
    if(!require("js/controls/TransformControls.js"))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load TransformControls.js";
    }
    if(!require("js/controls/DragControls.js"))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load DragControls.js";
    }

    onMove.connect(boost::bind(&WebUi::handleMove, this, _1, _2, _3, _4));
    onKeydown.connect(boost::bind(&WebUi::handleKeydown, this, _1));

    std::ifstream ifs;
    ifs.open("../render_script.txt");
    if(ifs.is_open())
    {
        std::string move_call = onMove.createCall("SELECTED.index", "SELECTED.position.x", 
            "SELECTED.position.y", "SELECTED.position.z");
        std::string source((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        std::stringstream ss;
        ss << source;
        ss << "\n";
        ss << "function onDocumentMouseUp( event ) {\n";
        ss << "  event.preventDefault();\n";
        ss << "  controls.enabled = true;\n";
        ss << "  if (INTERSECTED) {\n";
        ss <<        move_call;
        ss << "      SELECTED = null;\n";
        ss << "  }\n";
        ss << "  container.style.cursor = 'auto';\n";
        ss << "}\n";
        ss << "function onDocumentKeydown(event){\n";
        ss << "  var key = event.which; \n";
        ss <<    onKeydown.createCall("key");
        ss << "  if(key == 65){\n";
        ss << "    addBoundingBox();\n";
        ss << "  }\n";
        ss << "}\n";
        render_window = new Wt::WText(root());
        render_window->doJavaScript(ss.str());
    }   
}

WebUi::~WebUi()
{
    
}

void WebUi::handleMove(int index, float x, float y, float z)
{
    if(index >= 0 && index < g_sink->bounding_boxes.size())
    {
        g_sink->bounding_boxes[index].x = x;
        g_sink->bounding_boxes[index].y = y;
        g_sink->bounding_boxes[index].z = z;
    }
}

void WebUi::handleRotate(int index, float x, float y, float z)
{

}

void WebUi::handleResize(int index, float x, float y, float z)
{

}

void WebUi::handleActivate()
{
    g_sink->active_switch->UpdateData(!g_sink->active_switch->GetData());
}

void WebUi::handleAddbb()
{
    BoundingBox bb;
    bb.x = 0;
    bb.y = 0;
    bb.z = 0;
    bb.width = 20;
    bb.height = 30;
    bb.depth = 20;
    g_sink->bounding_boxes.push_back(bb);
    g_sink->bounding_boxes_param.Commit();
}

void WebUi::handleRebuildModel()
{
    auto stream = g_sink->GetDataStream();
    auto node = stream->GetNode("ForegroundEstimate0");
    if(node)
    {
        auto param = node->GetParameter<bool>("build_model");
        if(param)
        {
            param->UpdateData(true);
        }
    }
}

void WebUi::handleKeydown(int value)
{
    if(value == 32)
    {
        handleRebuildModel();
    }else if(value == 65)
    {
        handleAddbb();
    }else if(value == 187)
    {
        handleActivate();
    }

    std::cout << value << std::endl;
}

