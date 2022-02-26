#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H

#include "Random.h"
#include "JsonSerialisationHelper.h"

#include <nlohmann/json.hpp>
#include <fmt/format.h>

#include <vector>
#include <memory>

/**
 * A basic NeuralNetwork with no backward propogation. The sigma function
 * operates between 0.0 and 0.1.
 */
class NeuralNetwork {
public:
    // I don't usually like typedefs, but here we can collapse a complex 3D array into manageable types without creating a bunch of structs
    using InputWeight = double;
    using Node = std::vector<InputWeight>;
    using Layer = std::vector<Node>;

    enum class InitialWeights : bool {
        Random,
        PassThrough,
    };

    // FIXME work out how to not have this hard coded
    static constexpr unsigned BRAIN_WIDTH = 7;

    /**
     * Creates a rectangular network of the specified width and height, with
     * random edge weights between 0.0 and 1.0.
     */
    NeuralNetwork(unsigned layerCount, unsigned width, InitialWeights initialWeights);
    NeuralNetwork(std::vector<Layer>&& layers, unsigned width);

    static void ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<NeuralNetwork>& helper);

    size_t GetInputCount() const { return layers_.empty() ? 0 : layers_.front().size(); }
    size_t GetOutputCount() const { return layers_.empty() ? 0 : layers_.back().empty() ? 0 : layers_.back().size(); }
    size_t GetConnectionCount() const;

    /**
     * Inputs should be between 0.0 and 1.0 inclusive. Returns the final node
     * values.
     */
    void ForwardPropogate(std::vector<double>& inputs) const;

    void ForEach(const std::function<void(unsigned, unsigned, const Node&)>& perNode) const;
    size_t GetLayerWidth() const { return width_; }
    size_t GetLayerCount() const { return layers_.size(); }

    std::shared_ptr<NeuralNetwork> WithMutatedConnections() const;
    std::shared_ptr<NeuralNetwork> WithColumnAdded(size_t index, InitialWeights connections) const;
    std::shared_ptr<NeuralNetwork> WithColumnRemoved(size_t index) const;
    std::shared_ptr<NeuralNetwork> WithRowAdded(size_t index, InitialWeights connections) const;
    std::shared_ptr<NeuralNetwork> WithRowRemoved(size_t index) const;

private:
    static inline std::vector<double> previousNodeValues_;


    std::vector<Layer> layers_;
    size_t width_;

    static std::vector<Layer> CreateRandomLayers(size_t layerCount, size_t width);
    static Layer CreateRandomLayer(size_t width);
    static std::vector<Layer> CreatePassThroughLayers(size_t layerCount, size_t width);
    static Layer CreatePassThroughLayer(size_t width);

    std::vector<Layer> CopyLayers() const;
};

template<>
struct fmt::formatter<NeuralNetwork>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& context)
    {
        return context.begin();
    }

    template <typename FormatContext>
    auto format(const NeuralNetwork& network, FormatContext& context)
    {
        return fmt::format_to(context.out(), "{} inputs, {} layers", network.GetInputCount(), network.GetLayerCount());
    }
};

#endif // NEURALNETWORK_H
