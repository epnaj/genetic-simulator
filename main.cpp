#include <iostream>
#include <random>
#include <vector>
#include "brain.hpp"
#include "genetics.hpp"

int main(){
    std::cin.tie(NULL);
    std::ios_base::sync_with_stdio(NULL);
    double fitness = 0;
    constexpr unsigned int width = 20;
    constexpr unsigned int height = 15;
    constexpr unsigned int numberOfNeurons = 20;    // minimum is 6 neurons
    constexpr unsigned int totalNumberOfGenerations = 20000;
    constexpr unsigned int timeStepsPerGenerarion = 40;
    constexpr unsigned int numberOfCreatures = 20;
    typedef ContinuousNeuron myNeuronType;
    char board[height*width];
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution <> distWidth(0, width-1);
    std::uniform_int_distribution <> distHeight(0, height-1);
    std::uniform_int_distribution <> distSynapses(0, numberOfNeurons-1);
    std::uniform_real_distribution <> distZeroOne(0, 1);
	std::vector <unsigned int> arguments;
    arguments.push_back(width);
    arguments.push_back(height);
    std::vector <std::unique_ptr <Creature <myNeuronType>>> population(numberOfCreatures);
    std::vector <std::vector <unsigned int>> edges;
    std::vector <unsigned int> synapses(numberOfNeurons);
    for(auto &p : population){
        edges.clear();
        for(unsigned int i=0; i<numberOfNeurons; ++i){
            for(auto &s : synapses){
                s = distSynapses(gen);
            }
            edges.push_back(synapses);
        }
        p = std::make_unique <Creature <myNeuronType>>(Graph <unsigned int>(numberOfNeurons, edges), distWidth(gen), distHeight(gen));
    }
    // main loop
    for(int generation=0; generation<totalNumberOfGenerations; ++generation){
        for(auto &c : population){
            c->x = c->xBegin = distWidth(gen);
            c->y = c->yBegin = distHeight(gen);
        }
        for(int loop=0; loop<timeStepsPerGenerarion; ++loop){
            if(!((generation+1)%100)){
                clearAndfillBoard(board, width, height, population);
                printBoard(board, width, height, true, 50000);
                std::cout << "Generation : " << generation + 1 << ", previous best fitness : " << fitness << std::endl;
                std::cout << std::flush;
            }
            for(auto &c : population){
                getInput(c, population, width, height);
                // push forward in time step
                c->brain->forward();
                // number - 3 or/and -4 == optional stop neuron
                c->x += move(c->brain->neurons[numberOfNeurons-2], c->x, width, NULL);
                c->y += move(c->brain->neurons[numberOfNeurons-1], c->y, height, NULL);
            }
        }
        selection(population, arguments, [](std::unique_ptr<Creature <myNeuronType>> &crt, const std::vector<unsigned int> &args)->int{
                // make them gater in top left corner
                return (crt->x >= (args[0]/2)) + (crt->y <= (args[1]/2));
            }
        );
		reproduce(population, fitness, distSynapses, distZeroOne, gen);
    }
    char choose;
    clearScreenMacro();
    while(true){
        std::cout << "Do you want to watch creatures work? 0 - no, any other - yes." << std::endl;
        std::cin >> choose;
        if(choose == '0')
            break;
        for(auto &c : population){
            c->x = distWidth(gen);
            c->y = distHeight(gen);
        }
        for(int loop=0; loop<timeStepsPerGenerarion; ++loop){
            clearAndfillBoard(board, width, height, population);
            printBoard(board, width, height, true, 50000);
            for(auto &c : population){
                getInput(c, population, width, height);
                c->brain->forward();
                c->x += move(c->brain->neurons[numberOfNeurons-2], c->x, width, NULL);
                c->y += move(c->brain->neurons[numberOfNeurons-1], c->y, height, NULL);
            }
        }
    }

    return 0;
}