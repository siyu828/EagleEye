#pragma once
#define PARAMTERS_GENERATE_PERSISTENCE
#include <EagleLib/nodes/Node.h>
#include <MetaObject/MetaObject.hpp>
#include <MetaObject/Parameters/Types.hpp>

#include <caffe/solver.hpp>
#include <caffe/parallel.hpp>
namespace EagleLib
{
    namespace Nodes
    {
        class PLUGIN_EXPORTS caffe_solver: public Node
        {
        public:
            typedef std::vector<SyncedMemory> WrappedBlob_t;
            typedef std::map<std::string, WrappedBlob_t> BlobMap_t;
            enum LearningPolicies
            {
                fixed,
                step,
                exponential,
                inverse,
                multistep,
                poly,
                sigmoid
            };
            MO_DERIVE(caffe_solver, Node);
                PARAM(mo::ReadFile, solver_description, mo::ReadFile(""));
                PARAM(mo::ReadFile, network_description, mo::ReadFile(""));
                PARAM(mo::ReadFile, previous_solver_state, mo::ReadFile(""));
                PARAM(std::vector<mo::ReadFile>, weight_files, std::vector<mo::ReadFile>());
                PARAM(int, test_interval, 1000);
                PARAM(double, base_learning_rate, 0.001);
                PARAM(double, momentum, 0.9);
                PARAM(double, gamma, 0.1);
                PARAM(int, steps_per_iteration, 100);
                PARAM(int, step_size, 20000);
                PARAM(int, snapshot_interval, 10000);
                PARAM(std::string, snapshot_prefix, "snapshots/");
                PROPERTY(boost::shared_ptr<caffe::Net<float>>, neural_network, boost::shared_ptr<caffe::Net<float>>());
                //PROPERTY(std::vector<std::vector<std::vector<cv::Mat>>>, input_blobs, std::vector<std::vector<std::vector<cv::Mat>>>());
                OUTPUT(BlobMap_t, input_blobs, BlobMap_t());
                MO_SIGNAL(void, fill_blobs);
                MO_SIGNAL(void, update);
                PROPERTY(boost::shared_ptr<caffe::Solver<float>>, solver, boost::shared_ptr<caffe::Solver<float>>());
                ENUM_PARAM(solver_type, caffe::SolverParameter::SGD, caffe::SolverParameter::ADADELTA, caffe::SolverParameter::ADAGRAD, caffe::SolverParameter::ADAM, caffe::SolverParameter::NESTEROV, caffe::SolverParameter::RMSPROP)
                ENUM_PARAM(learning_rate_policy, step, fixed, exponential, inverse, multistep, poly, sigmoid);
            MO_END;
            bool ProcessImpl();
        };
        /*class PLUGIN_EXPORTS caffe_network: public Node
        {
        public:
            caffe_network();
            MO_DERIVE(caffe_network, Node)
                OUTPUT(boost::shared_ptr<caffe::Net<float>>, neural_network, NULL);
                OUTPUT(caffe_solver::BlobMap_t, input_blobs, )
                PARAM(mo::ReadFile, nn_description, mo::ReadFile(""));
            MO_END;
            virtual TS<SyncedMemory> doProcess(TS<SyncedMemory> input, cv::cuda::Stream& stream);
            virtual bool pre_check(const TS<SyncedMemory>& input);
        };*/
    }
}