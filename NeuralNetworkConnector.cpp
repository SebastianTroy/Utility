#include "NeuralNetworkConnector.h"

#include "Random.h"
#include "Algorithm.h"

#include <assert.h>

using namespace nlohmann;

NeuralNetworkConnector::NeuralNetworkConnector(unsigned inputs, unsigned outputs)
    : weights_(inputs, std::vector<double>(outputs, 0.0))
{
    std::vector<size_t> inputIndexes = util::CreateSeries<size_t>(0, inputs);
    std::vector<size_t> outputIndexes = util::CreateSeries<size_t>(0, outputs);

    // so we don't just connect input 1 to output 1, and input 2 to output 2 etc
    Random::Shuffle(inputIndexes);
    Random::Shuffle(outputIndexes);

    /*
     * Creates a bunch of 1:1 connections between inputs and outputs. The larger
     * of inputs or outputs will therefore have some unconnected nodes.
     */
    util::IterateBoth<size_t, size_t>(inputIndexes, outputIndexes, [&](const size_t& in, const size_t& out)
    {
        // set the weight of an input to an output to 1 so it is a "direct passthrough" connection
        weights_.at(in).at(out) = 1.0;
    });
}

NeuralNetworkConnector::NeuralNetworkConnector(std::vector<std::vector<double>>&& weights)
    : weights_(std::move(weights))
{
}

void NeuralNetworkConnector::ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<NeuralNetworkConnector>& helper)
{
    helper.RegisterConstructor(helper.CreateParameter("Weights", &NeuralNetworkConnector::weights_));
}

void NeuralNetworkConnector::PassForward(const std::vector<double>& inputValues, std::vector<double>& outputValues)
{
    assert(inputValues.size() == weights_.size() && outputValues.size() == weights_.at(0).size());
    util::IterateBoth<double, std::vector<double>>(inputValues, weights_, [&outputValues](const double& input, const std::vector<double>& inputWeights) -> void
    {
        util::IterateBoth<double, double>(inputWeights, outputValues, [&input](const double& inputWeight, double& output) -> void
        {
            output += input * inputWeight;
        });
    });
}

std::shared_ptr<NeuralNetworkConnector> NeuralNetworkConnector::WithMutatedConnections() const
{
    std::vector<std::vector<double>> newWeights = weights_;

    switch (Random::WeightedIndex({ 90, 8, 2 })) {
    case 0:
        // 90% Chance mutate a single connection
        if (newWeights.size() > 0) {
            size_t input = Random::Number<size_t>(size_t{ 0 }, newWeights.size() - 1);
            if (newWeights.at(input).size() > 0) {
                size_t output = Random::Number<size_t>(size_t{ 0 }, newWeights.at(input).size() - 1);
                newWeights.at(input).at(output) += Random::Gaussian(0.0, 0.2);
            }
        }
        break;
    case 1:
        // 8% Chance mutate all connections
        for (auto& input : newWeights) {
            for (auto& output : input) {
                output += Random::Gaussian(0.0, 0.1);
            }
        }
        break;
    case 2:
        // 2% Chance shuffle connections
        for (auto& input : newWeights) {
            Random::Shuffle(input);
        }
        break;
    }

    return std::make_shared<NeuralNetworkConnector>(std::move(newWeights));
}

std::shared_ptr<NeuralNetworkConnector> NeuralNetworkConnector::WithInputAdded(size_t index) const
{
    std::vector<std::vector<double>> newWeights = weights_;

    size_t newInputIndex = std::min(index, newWeights.size());
    auto newInputIter = newWeights.begin();
    std::advance(newInputIter, newInputIndex);
    newWeights.insert(newInputIter, std::vector<double>(GetOutputCount(), 0.0));

    return std::make_shared<NeuralNetworkConnector>(std::move(newWeights));
}

std::shared_ptr<NeuralNetworkConnector> NeuralNetworkConnector::WithInputRemoved(size_t index) const
{
    std::vector<std::vector<double>> newWeights = weights_;

    if (!newWeights.empty()) {
        size_t newInputIndex = std::min(index, newWeights.size() - 1);
        auto newInputIter = newWeights.begin();
        std::advance(newInputIter, newInputIndex);
        newWeights.erase(newInputIter);
    }

    return std::make_shared<NeuralNetworkConnector>(std::move(newWeights));
}

std::shared_ptr<NeuralNetworkConnector> NeuralNetworkConnector::WithOutputAdded(size_t index) const
{
    std::vector<std::vector<double>> newWeights = weights_;

    for (auto& connections : newWeights) {
        size_t newOutputIndex = std::min(index, connections.size());
        auto newOutputIter = connections.begin();
        std::advance(newOutputIter, newOutputIndex);
        connections.insert(newOutputIter, 0.0);
    }

    return std::make_shared<NeuralNetworkConnector>(std::move(newWeights));
}

std::shared_ptr<NeuralNetworkConnector> NeuralNetworkConnector::WithOutputRemoved(size_t index) const
{
    std::vector<std::vector<double>> newWeights = weights_;

    for (auto& connections : newWeights) {
        if (!connections.empty()) {
            size_t newOutputIndex = std::min(index, connections.size() - 1);
            auto newOutputIter = connections.begin();
            std::advance(newOutputIter, newOutputIndex);
            connections.erase(newOutputIter);
        }
    }

    return std::make_shared<NeuralNetworkConnector>(std::move(newWeights));
}
