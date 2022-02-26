#include "NeuralNetwork.h"

#include "Algorithm.h"

using namespace nlohmann;

NeuralNetwork::NeuralNetwork(unsigned layerCount, unsigned width, NeuralNetwork::InitialWeights initialWeights)
    : NeuralNetwork(initialWeights == InitialWeights::Random ? CreateRandomLayers(layerCount, width) : CreatePassThroughLayers(layerCount, width), width)
{
}

NeuralNetwork::NeuralNetwork(std::vector<NeuralNetwork::Layer>&& layers, unsigned width)
    : layers_(std::move(layers))
    , width_(width)
{
    for (auto& layer : layers_) {
        if(layer.size() != width_) {
            assert(layer.size() == width_);
        }
    }
}

void NeuralNetwork::ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<NeuralNetwork>& helper)
{
    helper.RegisterConstructor(helper.CreateParameter("Layers", &NeuralNetwork::layers_),
                               helper.CreateParameter("Width", &NeuralNetwork::width_)
                               );
}

size_t NeuralNetwork::GetConnectionCount() const
{
    size_t connectionCount = 0;
    for (const auto& layer : layers_) {
        if (layer.size() > 0) {
            connectionCount += layer.front().size() * layer.size();
        }
    }
    return connectionCount;
}

void NeuralNetwork::ForwardPropogate(std::vector<double>& toPropogate) const
{
    // about to swap with previousNodeValues so we can return outputs at the end
    // also allows to skip propogation when no hidden layers
    for (const auto& layer : layers_) {
        std::swap(toPropogate, previousNodeValues_);
        // We'll reuse this vector for the output of each layer
        toPropogate.assign(layer.size(), 0.0);

        size_t nodeIndex = 0;
        for (auto& node : layer) {
            double nodeValue = 0.0;
            size_t edgeIndex = 0;
            for (auto& edge : node) {
                nodeValue += edge * previousNodeValues_.at(edgeIndex);
                ++edgeIndex;
            }
            // tanh is our sigma function
            toPropogate.at(nodeIndex) = std::tanh(nodeValue);
            nodeIndex++;
        }
    }
}

std::shared_ptr<NeuralNetwork> NeuralNetwork::WithMutatedConnections() const
{
    std::vector<Layer> copy = CopyLayers();
    unsigned connectionCount = GetConnectionCount();

    for (auto& layer : copy) {
        for (auto& node : layer) {
            for (auto& edge : node) {
                // i.e average 3 mutations per child
                if (Random::PercentChance(300.0 / connectionCount)) {
                    edge += Random::Gaussian(0.0, 0.4);
                }
            }
        }
    }

    return std::make_shared<NeuralNetwork>(std::move(copy), width_);
}

std::shared_ptr<NeuralNetwork> NeuralNetwork::WithColumnAdded(size_t index, NeuralNetwork::InitialWeights connections) const
{
    std::vector<Layer> copy = CopyLayers();
    size_t newWidth = width_ + 1;
    index = std::min(index, width_);

    for (auto& layer : copy) {
        // First put in the new node with the old width
        auto layerIter = layer.begin();
        std::advance(layerIter, index);
        layer.insert(layerIter, Node(layer.size(), 0.0));
        // Then for each node in the layer, add a new connection
        for (auto& node : layer) {
            auto nodeIter = node.begin();
            std::advance(nodeIter, index);
            double newWeight = connections == InitialWeights::PassThrough ? 1.0 : Random::Gaussian(0.0, 0.4);
            node.insert(nodeIter, newWeight);
        }
    }

    return std::make_shared<NeuralNetwork>(std::move(copy), newWidth);
}

std::shared_ptr<NeuralNetwork> NeuralNetwork::WithColumnRemoved(size_t index) const
{
    std::vector<Layer> copy = CopyLayers();
    size_t newWidth = width_;

    if (width_ > 0) {
        newWidth -= 1;
        index = std::min(index, newWidth);

        for (auto& layer : copy) {
            // First take out the node
            auto layerIter = layer.begin();
            std::advance(layerIter, index);
            layer.erase(layerIter);
            // Then for each node in the layer, remove the connection
            for (auto& node : layer) {
                auto nodeIter = node.begin();
                std::advance(nodeIter, index);
                node.erase(nodeIter);
            }
        }
    }

    return std::make_shared<NeuralNetwork>(std::move(copy), newWidth);
}

std::shared_ptr<NeuralNetwork> NeuralNetwork::WithRowAdded(size_t index, NeuralNetwork::InitialWeights connections) const
{
    std::vector<Layer> copy = CopyLayers();

    index = std::min(index, copy.size());
    auto layersIter = copy.begin();
    std::advance(layersIter, index);
    copy.insert(layersIter, connections == InitialWeights::PassThrough ? CreatePassThroughLayer(width_) : CreateRandomLayer(width_));

    return std::make_shared<NeuralNetwork>(std::move(copy), width_);
}

std::shared_ptr<NeuralNetwork> NeuralNetwork::WithRowRemoved(size_t index) const
{
    std::vector<Layer> copy = CopyLayers();

    if (!copy.empty()) {
        index = std::min(index, copy.size());
        auto layersIter = copy.begin();
        std::advance(layersIter, index);
        copy.erase(layersIter);
    }

    return std::make_shared<NeuralNetwork>(std::move(copy), width_);
}

void NeuralNetwork::ForEach(const std::function<void (unsigned, unsigned, const NeuralNetwork::Node&)>& perNode) const
{
    unsigned layerIndex = 1;
    for (const auto& layer : layers_) {
        unsigned nodeIndex = 0;
        for (const auto& node : layer) {
            perNode(nodeIndex, layerIndex, node);
            nodeIndex++;
        }
        layerIndex++;
    }
}

std::vector<NeuralNetwork::Layer> NeuralNetwork::CreateRandomLayers(size_t layerCount, size_t width)
{
    std::vector<Layer> layers;
    layers.reserve(layerCount);
    for (unsigned i = 0; i < layerCount; ++i) {
        layers.push_back(CreateRandomLayer(width));
    }
    return layers;
}

NeuralNetwork::Layer NeuralNetwork::CreateRandomLayer(size_t width)
{
    Layer layer(width, Node(width));

    for (auto& node : layer) {
        double mean = 0.75;
        double stdDev = 0.25;
        node = Random::DualPeakGaussians(width, -mean, stdDev, mean, stdDev);
    }

    return layer;
}

std::vector<NeuralNetwork::Layer> NeuralNetwork::CreatePassThroughLayers(size_t layerCount, size_t width)
{
    std::vector<Layer> layers(layerCount, CreatePassThroughLayer(width));
    return layers;
}

NeuralNetwork::Layer NeuralNetwork::CreatePassThroughLayer(size_t width)
{
    Layer layer(width, Node(width));

    unsigned nodeColumn = 0;
    for (auto& node : layer) {
        for (unsigned inputColumn = 0; inputColumn < node.size(); inputColumn++) {
            node.at(inputColumn) = (inputColumn == nodeColumn ? 1.0 : 0.0);
        }
        nodeColumn++;
    }

    return layer;
}

std::vector<NeuralNetwork::Layer> NeuralNetwork::CopyLayers() const
{
    std::vector<Layer> copy(layers_.size(), Layer{});
    Tril::IterateBoth<Layer, Layer>(layers_, copy, [](const Layer& origLayer, Layer& copyLayer)
    {
        copyLayer.reserve(origLayer.size());
        for (const Node& origNode : origLayer ) {
            copyLayer.push_back(Node(origNode));
        }
    });
    return copy;
}
