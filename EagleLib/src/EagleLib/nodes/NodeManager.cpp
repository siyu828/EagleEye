#include "NodeManager.h"
#include "EagleLib/rcc/ObjectManager.h"
#include "Node.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
using namespace EagleLib;

NodeManager& NodeManager::getInstance()
{
	static NodeManager instance;
	return instance;
}

NodeManager::NodeManager()
{
	ObjectManager::Instance().RegisterConstructorAddedCallback(boost::bind(&NodeManager::OnConstructorsAdded, this));
}

NodeManager::~NodeManager()
{

}


void
NodeManager::OnConstructorsAdded()
{
	
	AUDynArray<IObjectConstructor*> constructors;
	ObjectManager::Instance().m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
	std::vector<Nodes::Node*> newNodes;
	for (size_t i = 0; i < constructors.Size(); ++i)
	{
		size_t numObjects = constructors[i]->GetNumberConstructedObjects();
		for (size_t j = 0; j < numObjects; ++j)
		{
			auto ptr = constructors[i]->GetConstructedObject(j);
			if (ptr)
			{
				ptr = ptr->GetInterface(IID_NodeObject);
				if (ptr)
				{
					auto nodePtr = static_cast<Nodes::Node*>(ptr);
					newNodes.push_back(nodePtr);
				}
			}
		}
	}
	for (size_t i = 0; i < newNodes.size(); ++i)
	{
		for (size_t j = 0; j < newNodes[i]->parameters.size(); ++j)
		{
			if (newNodes[i]->parameters[j]->type & Parameters::Parameter::Input)
			{
				auto inputParam = std::dynamic_pointer_cast<Parameters::InputParameter>(newNodes[i]->parameters[j]);
				inputParam->SetInput(std::string());
			}
		}
	}
}

shared_ptr<Nodes::Node> NodeManager::addNode(const std::string &nodeName)
{
	IObjectConstructor* pConstructor = ObjectManager::Instance().m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor(nodeName.c_str());

	if (pConstructor && pConstructor->GetInterfaceId() == IID_NodeObject)
	{
		IObject* pObj = pConstructor->Construct();
		IObject* interface = pObj->GetInterface(IID_NodeObject);

		if (interface)
		{
			Nodes::Node* node = static_cast<Nodes::Node*>(interface);
			try
			{
				node->Init(true);
			}
			catch (cv::Exception &e)
			{
				BOOST_LOG_TRIVIAL(error) << "Failed to initialize node " << nodeName << " due to: " << e.what();
				return shared_ptr<Nodes::Node>();
			}
			catch (...)
			{
				BOOST_LOG_TRIVIAL(error) << "Failed to initialize node " << nodeName;
				return shared_ptr<Nodes::Node>();
			}

			nodes.push_back(weak_ptr<Nodes::Node>(node));
			return Nodes::Node::Ptr(node);
		}
		else
		{
			BOOST_LOG_TRIVIAL(warning) << "[ NodeManager ] " << nodeName << " not a node";
			// Input nodename is a compatible object but it is not a node
			return shared_ptr<Nodes::Node>();
		}
	}
	else
	{
		BOOST_LOG_TRIVIAL(warning) << "[ NodeManager ] " << nodeName << " not a valid node name";
		return shared_ptr<Nodes::Node>();
	}

	return shared_ptr<Nodes::Node>();
}
std::vector<shared_ptr<Nodes::Node>> NodeManager::loadNodes(const std::string& saveFile)
{
	
	boost::filesystem::path path(saveFile);
	if (!boost::filesystem::is_regular_file(path))
	{
		//std::cout << "Unable to load " << saveFile << " doesn't exist, or is not a regular file" << std::endl;
		LOG_TRIVIAL(warning) << "[ NodeManager ] " << saveFile << " doesn't exist or not a regular file";
	}
	cv::FileStorage fs;
	try
	{
		fs.open(saveFile, cv::FileStorage::READ);
	}
	catch (cv::Exception &e)
	{
		//std::cout << e.what() << std::endl;
		LOG_TRIVIAL(error) << "[ NodeManager ] " << e.what();
	}

	int nodeCount = (int)fs["TopLevelNodeCount"];
	LOG_TRIVIAL(info) << "[ NodeManager ] " << "Loading " << nodeCount << " nodes";
	std::vector<shared_ptr<Nodes::Node>> nodes;
	nodes.reserve(nodeCount);
	for (int i = 0; i < nodeCount; ++i)
	{
		auto nodeFS = fs["Node-" + boost::lexical_cast<std::string>(i)];
		std::string name = (std::string)nodeFS["NodeName"];
		Nodes::Node::Ptr node = addNode(name);
		node->Init(nodeFS);
		nodes.push_back(node);
	}
	return nodes;
}

void NodeManager::saveNodes(std::vector<shared_ptr<Nodes::Node>>& topLevelNodes, const std::string& fileName)
{
	cv::FileStorage fs;
	fs.open(fileName, cv::FileStorage::WRITE);
	saveNodes(topLevelNodes, fs);
	fs.release();
}
void NodeManager::saveNodes(std::vector<shared_ptr<Nodes::Node>>& topLevelNodes, cv::FileStorage fs)
{
	
	fs << "TopLevelNodeCount" << (int)topLevelNodes.size();

	for (size_t i = 0; i < topLevelNodes.size(); ++i)
	{
		fs << "Node-" + boost::lexical_cast<std::string>(i) << "{";
		topLevelNodes[i]->Serialize(fs);
		fs << "}";
	}
}

bool NodeManager::removeNode(const std::string& nodeName)
{
	
	return false;
}
std::string NodeManager::getNodeFile(const ObjectId& id)
{
	
	AUDynArray<IObjectConstructor*> constructors;
	ObjectManager::Instance().m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
	if (constructors.Size() > id.m_ConstructorId)
	{
		return std::string(constructors[id.m_ConstructorId]->GetFileName());
	}
	return std::string();
}

bool NodeManager::removeNode(ObjectId oid)
{
	
	return false;
}
void NodeManager::RegisterNodeInfo(const char* nodeName, std::vector<char const*>& nodeInfo)
{
	m_nodeInfoMap[nodeName] = nodeInfo;
}
std::vector<const char*> NodeManager::GetNodeInfo(std::string& nodeName)
{
    auto constructor = ObjectManager::Instance().m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor(nodeName.c_str());
    if (constructor)
    {
        auto obj_info = constructor->GetObjectInfo();
        if (obj_info)
        {
            if (obj_info->GetObjectInfoType() == 1)
            {
                auto node_info = dynamic_cast<EagleLib::Nodes::NodeInfo*>(obj_info);
                if (node_info)
                {
                    return node_info->GetNodeHierarchy();
                }
                
            }
        }
    }
    return std::vector<const char*>();
	//return m_nodeInfoMap[nodeName];
}






void NodeManager::saveTree(const std::string &fileName)
{
	
}

void
NodeManager::onNodeRecompile(Nodes::Node *node)
{
}

Nodes::Node*
NodeManager::getNode(const ObjectId& id)
{
	
	AUDynArray<IObjectConstructor*> constructors;
	ObjectManager::Instance().m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
	if (!id.IsValid())
		return nullptr;
	if (id.m_ConstructorId >= constructors.Size())
		return nullptr;
	if (id.m_PerTypeId >= constructors[id.m_ConstructorId]->GetNumberConstructedObjects())
		return nullptr;
	IObject* pObj = constructors[id.m_ConstructorId]->GetConstructedObject(id.m_PerTypeId);
	if (!pObj)
		return nullptr;
	pObj = pObj->GetInterface(IID_NodeObject);
	if (!pObj)
		return nullptr;
	return static_cast<Nodes::Node*>(pObj);
}

Nodes::Node*
NodeManager::getNode(const std::string &treeName)
{
	
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i] != nullptr)
		{
			if (nodes[i]->fullTreeName == treeName)
			{
				return nodes[i].get();
			}
		}
	}
	return nullptr;
}

void
NodeManager::updateTreeName(Nodes::Node* node, const std::string& prevTreeName)
{
	
	
}


void
NodeManager::getSiblingNodes(const std::string& sourceNode, std::vector<Nodes::Node*>& output)
{
	
}

void printTreeHelper(std::stringstream& tree, int level, Nodes::Node* node)
{
	
	for (int i = 0; i < level; ++i)
	{
		tree << "+";
	}
	tree << node->getFullTreeName() << std::endl;
	for (size_t i = 0; i < node->children.size(); ++i)
	{
		printTreeHelper(tree, level + 1, node->children[i].get());
	}
}

void NodeManager::printNodeTree(std::string* ret)
{
	
	std::stringstream tree;
	std::vector<weak_ptr<Nodes::Node>> parentNodes;
	// First get the top level nodes for the tree
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i] != nullptr)
		{
			if (nodes[i]->parent == nullptr)
			{
				parentNodes.push_back(nodes[i]);
			}
		}
	}
	for (size_t i = 0; i < parentNodes.size(); ++i)
	{
		printTreeHelper(tree, 0, parentNodes[i].get());
	}
	if (ret)
	{
		*ret = tree.str();
	}
	else
	{
		std::cout << tree.str() << std::endl;
	}
}

Nodes::Node*
NodeManager::getParent(const std::string& sourceNode)
{
	
	return nullptr;
}
void NodeManager::getParentNodes(const std::string& sourceNode, std::vector<Nodes::Node*>& output)
{
	
}

void NodeManager::getAccessibleNodes(const std::string& sourceNode, std::vector<Nodes::Node*>& output)
{
	
	getSiblingNodes(sourceNode, output);
	getParentNodes(sourceNode, output);
}
std::vector<std::string>
NodeManager::getConstructableNodes()
{
	
	AUDynArray<IObjectConstructor*> constructors;
	ObjectManager::Instance().m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
	std::vector<std::string> output;
	for (size_t i = 0; i < constructors.Size(); ++i)
	{
		if (constructors[i])
		{
			if (constructors[i]->GetInterfaceId() == IID_NodeObject)
				output.push_back(constructors[i]->GetName());
		}
		else
		{
			std::cout << "Null constructor idx " << i << std::endl;
		}
	}
	return output;
}

std::vector<std::string> NodeManager::getParametersOfType(boost::function<bool(Loki::TypeInfo)> selector)
{
	
	std::vector<std::string> parameters;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		for (size_t j = 0; j < nodes[i]->parameters.size(); ++j)
		{
			if (selector(nodes[i]->parameters[j]->GetTypeInfo()))
				parameters.push_back(nodes[i]->parameters[j]->GetTreeName());
		}
	}
	return parameters;
}
