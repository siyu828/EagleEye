#pragma once

#include "nodes/Node.h"
//#include <opencv2/highgui.hpp>
//RUNTIME_COMPILER_LINKLIBRARY("-lopencv_core -lopencv_highgui")
#define PARAMTESTMACRO(type) updateParameter< type >(##type,0)
namespace EagleLib
{
    class TestNode: public Node
    {
    public:
        TestNode():Node()
        {
            nodeName = "TestNode";
            treeName = nodeName;
            //parent = nullptr;
        }

        ~TestNode(){}
        void Init(bool firstInit)
        {
            typedef std::vector<std::pair<int,double>> vec_ID;
            typedef std::vector<std::pair<double,double>> vec_DD;
            typedef std::vector<std::pair<double,int>> vec_DI;
            typedef std::vector<std::pair<int, int>> vec_II;
            updateParameter<int>("int",0);
            updateParameter<short>("short",0);
            updateParameter<char>("char",0);
            updateParameter<unsigned int>("unsigned int",0);
            updateParameter<unsigned short>("unsigned short",0);
            updateParameter<unsigned char>("unsigned char",0);

            updateParameter<int*>("int*",0);
            updateParameter<short*>("short*",0);
            updateParameter<char*>("char*",0);
            updateParameter<unsigned int*>("unsigned int*",0);
            updateParameter<unsigned short*>("unsigned short*",0);
            updateParameter<unsigned char*>("unsigned char*",0);

            updateParameter<float>("float",0);
            updateParameter<double>("double",0);
            updateParameter<float*>("float*",0);
            updateParameter<double*>("double*",0);

            updateParameter<std::string>("std::string","test string");
            updateParameter<std::string*>("std::string*",nullptr);


            updateParameter<boost::function<void(void)>>("Function", boost::bind(&TestNode::testFunction,this));

            updateParameter<cv::Scalar>("cv::Scalar", cv::Scalar());
            updateParameter<std::vector<int>>("std::vector<int>", std::vector<int>());
            getParameter<std::vector<int>>("std::vector<int>")->data.push_back(10);
            getParameter<std::vector<int>>("std::vector<int>")->data.push_back(15);
            updateParameter<std::vector<double>>("std::vector<double>", std::vector<double>());
            getParameter<std::vector<double>>("std::vector<double>")->data.push_back(10.5);
            getParameter<std::vector<double>>("std::vector<double>")->data.push_back(15.5);

            vec_ID vecID;
            vecID.push_back(std::pair<int,double>(0, 0.5));
            vecID.push_back(std::pair<int,double>(1, 0.6));
            vecID.push_back(std::pair<int,double>(2, 0.7));
            updateParameter<vec_ID>("std::vector<std::pair<int,double>> control", vecID);
            updateParameter<vec_ID>("std::vector<std::pair<int,double>> state", vecID, Parameter::State);
            vec_DD vecDD;
            vecID.push_back(std::pair<double,double>(0, 0.5));
            vecID.push_back(std::pair<double,double>(1, 0.6));
            vecID.push_back(std::pair<double,double>(2, 0.7));
            updateParameter<vec_DD>("std::vector<std::pair<double,double>> control", vecDD);
            updateParameter<vec_DD>("std::vector<std::pair<double,double>> state", vecDD, Parameter::State);

            vec_II vecII;
            vecII.push_back(std::pair<int,int>(0, 3));
            vecII.push_back(std::pair<int,int>(1, 2));
            vecII.push_back(std::pair<int,int>(2, 1));
            updateParameter<vec_II>("std::vector<std::pair<int,int>> control", vecII);
            updateParameter<vec_II>("std::vector<std::pair<int,int>> state", vecII, Parameter::State);

            vec_DI vecDI;
            vecDI.push_back(std::pair<double, int>(0.5, 3));
            vecDI.push_back(std::pair<double, int>(1.2, 2));
            vecDI.push_back(std::pair<double, int>(2.3, 1));
            updateParameter<vec_DI>("std::vector<std::pair<double,int>> control", vecDI, Parameter::State);
            updateParameter<vec_DI>("std::vector<std::pair<double,int>> state", vecDI, Parameter::State);

            updateParameter<int>("int state",0, Parameter::State);
            updateParameter<short>("short state",0, Parameter::State);
            updateParameter<char>("char state",0, Parameter::State);
            updateParameter<unsigned int>("unsigned int state",0, Parameter::State);
            updateParameter<unsigned short>("unsigned short state",0, Parameter::State);
            updateParameter<unsigned char>("unsigned char state",0, Parameter::State);

            updateParameter<int*>("int* state",0, Parameter::State);
            updateParameter<short*>("short* state",0, Parameter::State);
            updateParameter<char*>("char* state",0, Parameter::State);
            updateParameter<unsigned int*>("unsigned int* state",0, Parameter::State);
            updateParameter<unsigned short*>("unsigned short* state",0, Parameter::State);
            updateParameter<unsigned char*>("unsigned char* state",0, Parameter::State);

            updateParameter<float>("float state",0, Parameter::State);
            updateParameter<double>("double state",0, Parameter::State);
            updateParameter<float*>("float* state",0, Parameter::State);
            updateParameter<double*>("double* state",0, Parameter::State);

            updateParameter<std::string>("std::string state","test string", Parameter::State);
            updateParameter<std::string*>("std::string* state",nullptr, Parameter::State);

            updateParameter<cv::Mat>("cv::Mat state", cv::Mat(), Parameter::State);
            updateParameter<cv::Scalar>("cv::Scalar state", cv::Scalar(), Parameter::State);
            updateParameter<std::vector<int>>("std::vector<int> state", std::vector<int>(), Parameter::State);
            getParameter<std::vector<int>>("std::vector<int> state")->data.push_back(11);
            getParameter<std::vector<int>>("std::vector<int> state")->data.push_back(12);
            updateParameter<std::vector<double>>("std::vector<double> state", std::vector<double>(), Parameter::State);
            getParameter<std::vector<double>>("std::vector<double> state")->data.push_back(11.5);
            getParameter<std::vector<double>>("std::vector<double> state")->data.push_back(12.5);
            updateParameter<int>("int output",0, Parameter::Output);
            updateParameter<short>("short output",0, Parameter::Output);
            updateParameter<char>("char output",0, Parameter::Output);
            updateParameter<unsigned int>("unsigned int output",0, Parameter::Output);
            updateParameter<unsigned short>("unsigned short output",0, Parameter::Output);
            updateParameter<unsigned char>("unsigned char output",0, Parameter::Output);

            updateParameter<int*>("int* output",0, Parameter::Output);
            updateParameter<short*>("short* output",0, Parameter::Output);
            updateParameter<char*>("char* output",0, Parameter::Output);
            updateParameter<unsigned int*>("unsigned int* output",0, Parameter::Output);
            updateParameter<unsigned short*>("unsigned short* output",0, Parameter::Output);
            updateParameter<unsigned char*>("unsigned char* output",0, Parameter::Output);

            updateParameter<float>("float output",0, Parameter::Output);
            updateParameter<double>("double output",0, Parameter::Output);
            updateParameter<float*>("float* output",0, Parameter::Output);
            updateParameter<double*>("double* output",0, Parameter::Output);

            updateParameter<std::string>("std::string output","test string", Parameter::Output);
            updateParameter<std::string*>("std::string* output",nullptr, Parameter::Output);

            updateParameter<cv::Mat>("cv::Mat output", cv::Mat(), Parameter::Output);

            updateParameter<int>("int input",0, Parameter::Input);
            updateParameter<short>("short input",0, Parameter::Input);
            updateParameter<char>("char input",0, Parameter::Input);
            updateParameter<unsigned int>("unsigned int input",0, Parameter::Input);
            updateParameter<unsigned short>("unsigned short input",0, Parameter::Input);
            updateParameter<unsigned char>("unsigned char input",0, Parameter::Input);

            updateParameter<int*>("int* input",0, Parameter::Input);
            updateParameter<short*>("short* input",0, Parameter::Input);
            updateParameter<char*>("char* input",0, Parameter::Input);
            updateParameter<unsigned int*>("unsigned int* input",0, Parameter::Input);
            updateParameter<unsigned short*>("unsigned short* input",0, Parameter::Input);
            updateParameter<unsigned char*>("unsigned char* input",0, Parameter::Input);

            updateParameter<float>("float input",0, Parameter::Input);
            updateParameter<double>("double input",0, Parameter::Input);
            updateParameter<float*>("float* input",0, Parameter::Input);
            updateParameter<double*>("double* input",0, Parameter::Input);

            updateParameter<std::string>("std::string input","test string", Parameter::Input);
            updateParameter<std::string*>("std::string* input",nullptr, Parameter::Input);
        }

        cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img)
        {
            std::cout << "test" << std::endl;
            //cv::imshow("Test", cv::Mat(img));
            return img;
        }
        void testFunction()
        {
            log(Status, "Test function called");
        }
        //boost::function<void(void)> function;

    };


}
using namespace EagleLib;
REGISTERCLASS(TestNode)

