#include "nodes/Node.h"
#include <opencv2/highgui.hpp>
#include <regex>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "Manager.h"
using namespace EagleLib;
#ifdef RCC_ENABLED
#include "../RuntimeObjectSystem/ObjectInterfacePerModule.h"
#include "../RuntimeObjectSystem/ISimpleSerializer.h"

#if _WIN32
	#if _DEBUG
		RUNTIME_COMPILER_LINKLIBRARY("opencv_core300d.lib")
		RUNTIME_COMPILER_LINKLIBRARY("opencv_cuda300d.lib")
	#else
		RUNTIME_COMPILER_LINKLIBRARY("opencv_core300.lib")
		RUNTIME_COMPILER_LINKLIBRARY("opencv_cuda300.lib")
	#endif
#else
RUNTIME_COMPILER_LINKLIBRARY("-lopencv_core -lopencv_cuda")
#endif



#endif

Node::Node()
{
	treeName = nodeName;
    enabled = true;
	externalDisplay = false;
	drawResults = false;
}

Node::~Node()
{
    NodeManager::getInstance().onNodeRecompile(this);
}
void
Node::getInputs()
{

}
Node*
Node::addChild(Node* child)
{
    if (!child)
        return child;
    if(errorCallback)
        child->errorCallback = errorCallback;
    if(statusCallback)
        child->statusCallback = statusCallback;
    if(warningCallback)
        child->warningCallback = warningCallback;

    int count = children.get<NodeName>().count(child->nodeName);

	std::string prevTreeName = child->fullTreeName;
    child->setParent(fullTreeName, GetObjectId());
	child->setTreeName(child->nodeName + "-" + boost::lexical_cast<std::string>(count));
	
	// Notify the node manager of the tree name
	NodeManager::getInstance().updateTreeName(child, prevTreeName);


    NodeInfo info;
    info.id = child->GetObjectId();
    info.index = children.get<0>().size();
    info.nodeName = child->nodeName;
    info.treeName = child->treeName;
    children.get<0>().push_back(info);
    return child;
}


Node*
Node::getChild(const std::string& treeName)
{
    auto itr = children.get<TreeName>().find(treeName);
    if(itr == children.get<TreeName>().end())
        return nullptr;
    return NodeManager::getInstance().getNode(itr->id);
}


Node*
Node::getChild(const int& index)
{
    auto itr = children.get<0>()[index];
	return NodeManager::getInstance().getNode(itr.id);
}

Node*
Node::getChild(const ObjectId& id)
{
	return NodeManager::getInstance().getNode(id);
}

boost::shared_ptr<Parameter> 
Node::getParameter(int idx)
{
	return parameters[idx];
}

boost::shared_ptr<Parameter> 
Node::getParameter(const std::string& name)
{
	for (int i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i]->name == name)
			return parameters[i];
	}
	return boost::shared_ptr<Parameter>();
}
std::vector<std::string> Node::listParameters()
{
	std::vector<std::string> paramList;
	for (int i = 0; i < parameters.size(); ++i)
	{
		paramList.push_back(parameters[i]->name);
	}
	return paramList;
}
std::vector<std::string> Node::listInputs()
{
	std::vector<std::string> paramList;
	for (int i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i]->type & Parameter::Input)
			paramList.push_back(parameters[i]->name);
	}
	return paramList;
}
Node*
Node::getChildRecursive(std::string treeName_)
{

    // TODO tree structure parsing and correct directing of the search
    // Find the common base between this node and treeName


    return nullptr;
}

void
Node::removeChild(ObjectId childId)
{


}

void
Node::removeChild(const std::string &name)
{
    auto itr = children.get<NodeName>().find(name);
    if(itr != children.get<NodeName>().end())
        children.get<NodeName>().erase(itr);

}

cv::cuda::GpuMat
Node::process(cv::cuda::GpuMat &img)
{
    if(img.empty() && SkipEmpty())
        return img;
    try
    {
        if(enabled)
            return doProcess(img);
    }catch(cv::Exception &err)
    {
        log(Error, err.what());
    }catch(std::exception &err)
    {
        log(Error, err.what());
    }
    return img;
}
void					
Node::process(cv::InputArray in, cv::OutputArray out)
{

	try
	{
		return doProcess(in, out);
	}
    catch (cv::Exception &err)
	{
        log(Error, err.what());
	}
}

cv::cuda::GpuMat
Node::doProcess(cv::cuda::GpuMat& img)
{
    return img;
}
void					
Node::doProcess(cv::InputArray, cv::OutputArray)
{

}

void
Node::doProcess(cv::cuda::GpuMat& img, boost::promise<cv::cuda::GpuMat> &retVal)
{
    retVal.set_value(process(img));
}
void
Node::doProcess(cv::InputArray in, boost::promise<cv::OutputArray> &retVal)
{
	// Figure this out later :(

	
}
void
Node::registerDisplayCallback(boost::function<void(cv::Mat)>& f)
{
    cpuDisplayCallback = f;
}

void
Node::registerDisplayCallback(boost::function<void(cv::cuda::GpuMat)>& f)
{
	gpuDisplayCallback = f;
}

void
Node::spawnDisplay()
{
	cv::namedWindow(treeName);
	externalDisplay = true;
}
void
Node::killDisplay()
{
	if (externalDisplay)
		cv::destroyWindow(treeName);
}
std::string
Node::getName() const
{
    return nodeName;
}
std::string
Node::getTreeName() const
{
    return treeName;
}
Node* Node::getParent()
{
    if(parentId.IsValid())
        return NodeManager::getInstance().getNode(parentId);
    if(parentName.size())
        return NodeManager::getInstance().getNode(parentName);
    return nullptr;
}


Node*
Node::swap(Node* other)
{
    // By moving ownership of all parameters to the new node, all

    return other;
}
void
Node::Init(bool firstInit)
{
    if(firstInit)
        m_OID = GetObjectId();
}

void
Node::Init(const std::string &configFile)
{

}

void
Node::Init(const cv::FileNode& configNode)
{

}

void
Node::Serialize(ISimpleSerializer *pSerializer)
{
    IObject::Serialize(pSerializer);
    std::cout << "Serializing node" << std::endl;
    //SERIALIZE(children);
    SERIALIZE(parameters);
    SERIALIZE(children);
    SERIALIZE(treeName);
    SERIALIZE(nodeName);
	SERIALIZE(fullTreeName);
}
std::vector<std::string>
Node::findType(Loki::TypeInfo &typeInfo)
{
	std::vector<Node*> nodes;
	NodeManager::getInstance().getAccessibleNodes(fullTreeName, nodes);
    return findType(typeInfo, nodes);
}

std::vector<std::string> 
Node::findType(Loki::TypeInfo &typeInfo, std::vector<Node*>& nodes)
{
	std::vector<std::string> output;

	for (int i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i] == this)
			continue;
		for (int j = 0; j < nodes[i]->parameters.size(); ++j)
		{
            if (nodes[i]->parameters[j]->typeInfo == typeInfo && nodes[i]->parameters[j]->type & Parameter::Output)
				output.push_back(nodes[i]->parameters[j]->treeName);
		}
	}
	return output;
}
std::vector<std::vector<std::string>> 
Node::findCompatibleInputs()
{
	std::vector<std::vector<std::string>> output;
	for (int i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i]->type & Parameter::Input)
            output.push_back(findType(parameters[i]->typeInfo));
	}
	return output;
}
void
Node::setInputParameter(std::string sourceName, std::string inputName)
{
	for (int i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i]->name == inputName && parameters[i]->type & Parameter::Input)
			parameters[i]->setSource(sourceName);
	}
}

void
Node::setInputParameter(std::string sourceName, int inputIdx)
{
	int count = 0;
	for (int i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i]->type & Parameter::Input)
		{
			if (count == inputIdx)
				parameters[i]->setSource(sourceName);
			++count;
		}
	}
}
void
Node::setTreeName(const std::string& name)
{
	treeName = name;
    Node* parentPtr = NodeManager::getInstance().getNode(parentName);
	std::string fullTreeName_;
	if (parentPtr)
		fullTreeName_ = parentPtr->fullTreeName + "." + treeName;
	else
		fullTreeName_ = treeName;
	setFullTreeName(fullTreeName_);

}
void
Node::setFullTreeName(const std::string& name)
{
	for (int i = 0; i < parameters.size(); ++i)
	{
		parameters[i]->treeName = name + ":" + parameters[i]->name;
	}
	fullTreeName = name;
}

void
Node::setParent(const std::string& name, const ObjectId& parentId_)
{
    parentName = name;
    parentId = parentId_;
}
void 
Node::updateInputParameters()
{
	for (int i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i]->type & EagleLib::Parameter::Input)
		{
			parameters[i]->setSource("");
		}
	}
}
bool Node::SkipEmpty() const
{
    return true;
}
void Node::log(Verbosity level, const std::string &msg)
{
    switch(level)
    {
    case Profiling:
        if(profilingCallback)
            return profilingCallback(msg, this);
    case Status:
        if(statusCallback)
            return statusCallback(msg, this);
        std::cout << "[ " << fullTreeName << " - STATUS ]" << msg << std::endl;
    case Warning:
        if(warningCallback)
            return warningCallback(msg,this);
        std::cout << "[ " << fullTreeName << " - WARNING ]" << msg << std::endl;
    case Error:
        if(errorCallback)
            return errorCallback(msg,this);
        std::cout << "[ " << fullTreeName << " - ERROR ]" << msg << std::endl;
    case Critical:
        if(errorCallback)
            return errorCallback(msg,this);
        std::cout << "[ " << fullTreeName << " - CRITICAL ]" << msg << std::endl;
    }


}

REGISTERCLASS(Node)
