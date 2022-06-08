#ifndef brain_hpp
#define brain_hpp

#include <iostream>
#include <memory>
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iomanip>

template <typename T>
struct Graph{
    unsigned int vertices;
    std::vector <std::vector <T>> edges;
    Graph(const unsigned int &V, const std::vector<std::vector <T>> &E) : vertices(V), edges(E) {}
};

template <typename T>
struct Neuron{
    T value;
    T bias;
    std::unique_ptr <T[]> synapses;
    unsigned int s_size;
    Neuron(const unsigned int &numberOfSynapses) : bias(0), value(0), s_size(numberOfSynapses) {
        synapses = std::make_unique <T[]>(numberOfSynapses);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution <> dist(-1, 1);
        bias = dist(gen);
        for(unsigned int i=0; i<s_size; ++i){
            synapses[i] = dist(gen);
        }
    }
    virtual void activate() = 0;
    virtual void updateValue(const unsigned int &index, const T &input) = 0;
};

struct DiscreteNeuron : virtual public  Neuron <float> {
    using Neuron::Neuron;
    void print() { std::cout << value << std::endl; }
    void activate() override {
        value = value < 0 ? -1 : float(value > 0);
    }
    void updateValue(const unsigned int &index, const float &input) override {
        value += input*synapses[index] + bias;
    }
};

struct ContinuousNeuron : virtual public Neuron <double> {
    using Neuron::Neuron;
    void print() { std::cout << value << std::endl; }
    void activate() override {
        value = 2/(1+exp(-value)) - 1;
    }
    void updateValue(const unsigned int &index, const double &input) override {
        value += input*synapses[index] + bias;
    }
};

template <typename neuronType>
struct Brain{
    std::vector <std::unique_ptr<neuronType>> neurons;
    std::vector <bool> chromosome;
    Brain(const Graph <unsigned int> &brainStructure){
        chromosome.resize(brainStructure.vertices*brainStructure.vertices, false);
        for(unsigned int i=0; i<brainStructure.vertices; ++i){
            neurons.push_back(std::make_unique <neuronType> (brainStructure.edges[i].size()));
            int k = 0;
            for(auto &e : brainStructure.edges[i])
                chromosome[i*brainStructure.vertices + e] = true;
        }
    }
    void print(){
        std::cout << std::fixed << std::setprecision(3);
        for(unsigned int i=0; i<neurons.size(); ++i){
            std::cout << neurons[i]->value << " ";
            for(unsigned int k=0; k<neurons.size(); ++k){
                if(chromosome[i*neurons.size() + k]){
                    std::cout << k/neurons.size() + k%neurons.size() << " ";
                }
            }
            std::cout << std::endl;
        }
    }
    void forward(){
        for(unsigned int i=0; i<neurons.size(); ++i){
            for(unsigned int j=0; j<neurons.size(); ++j){
                neurons[i]->updateValue(j, chromosome[i*neurons.size() +j]*neurons[i]->value);
            }
            neurons[i]->activate();
        }
    }
};

struct coordinates{
    int x;
    int y;
};

template <typename neuronType>
struct Creature : virtual public coordinates{
    std::unique_ptr<Brain <neuronType>> brain;
    int xBegin;
    int yBegin;
    int fitness;
    Creature(const Graph <unsigned int> &brainStructure, const int &newX = 1, const int &newY = 1){
        x = newX, y = newY;
        brain = std::make_unique<Brain <neuronType>> (brainStructure);
    }
};

#endif