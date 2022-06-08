#ifndef genetics_hpp
#define genetics_hpp
#include "brain.hpp"
#include <iostream>
#include <functional>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>

// in out functions

void clearScreenMacro(){
#ifdef _WIN32
	system("cls");
#endif
#ifdef __linux__
	system("clear");
#endif
#ifdef __APPLE__
	system("clear")
#endif
}

void printBoard(const char *b, const unsigned int &w, const unsigned int &h, const bool &clearScreen, const unsigned int &waitForMicroseconds){
	std::this_thread::sleep_for(std::chrono::microseconds(waitForMicroseconds));
	if(clearScreen)
		clearScreenMacro();
	std::string s(w+2, '#');
	std::cout << s << std::endl;
	for(unsigned int i=0; i<h; ++i){
		std::cout << '#';
		for(unsigned int j=0; j<w; ++j){
			std::cout << b[i*w +j];
		}
		std::cout << '#' << std::endl;
	}
	std::cout << s << std::endl;
}

template <typename neuronType>
void clearAndfillBoard(char *b, const unsigned int &w, const unsigned int &h, const std::vector< std::unique_ptr <Creature <neuronType>>> &population){
	for(unsigned int i=0; i<h; ++i){
		for(unsigned int j=0; j<w; ++j){
			b[i*w + j] = ' ';
		}
	}
	for(auto &c : population){
		b[c->y*w + c->x] = 'x';
	}
}

template <typename neuronType>
void getInput(std::unique_ptr< Creature <neuronType>> &c, std::vector< std::unique_ptr <Creature <neuronType>>> &population, const unsigned int &w, const unsigned int &h){
	// assume smallest neuron number is 6
	// and 2 last neurons are always output neurons where // 0 - x axis, 1 - y axis (if only we use discrete neurons, in continuous version u have to use tanh or 2*sig() - 1)
	for(unsigned int i=0; i<population.size(); ++i){
		population[i]->brain->neurons[0]->value = population[i]->x/w;
		population[i]->brain->neurons[1]->value = population[i]->y/h;
		population[i]->brain->neurons[2]->value = doesAlignWithAnotherCreatureX(population, population[i]->x) * 2 - 1;
		population[i]->brain->neurons[3]->value = doesAlignWithAnotherCreatureY(population, population[i]->y) * 2 - 1;
        // you may add any inputs here as long ans you set correct number of neurons for each creature
	}
}

template <typename fpType>
int move(std::unique_ptr <DiscreteNeuron> &n, const int &currentPos, const unsigned int &bound, const fpType &canIMove){
	if(canIMove > 0)
		return 0;
	int nextPos = currentPos + n->value;
	if(0 <= nextPos && nextPos < bound)
		return n->value;
	return 0;
}

template <typename fpType>
int move(std::unique_ptr <ContinuousNeuron> &n, const int &currentPos, const unsigned int &bound, const fpType &canIMove){
	if(canIMove > 0)
		return 0;
	int nextPos = currentPos + (n->value > 0.0 ? 1.0 : (n->value == 0.0 ? 0.0 : -1.0) );
	if(0 <= nextPos && nextPos < bound)
		return nextPos - currentPos;
	return 0;
}

template <typename neuronType>
bool doesAlignWithAnotherCreatureX(const std::vector <std::unique_ptr <Creature <neuronType>>> &population, const unsigned int &x){
	for(auto &c : population){
		if(c->x == x)
			return true;
	}
	return false;
}

template <typename neuronType>
bool doesAlignWithAnotherCreatureY(const std::vector <std::unique_ptr <Creature <neuronType>>> &population, const unsigned int &y){
	for(auto &c : population){
		if(c->y == y)
			return true;
	}
	return false;
}

// mutations, time evolution

template <typename neuronType, typename argsType, typename functionCondition>
void selection(std::vector <std::unique_ptr< Creature <neuronType>>> &population, std::vector<argsType> &args, functionCondition function){
	for(auto &p : population){
        p->fitness = function(p, args);
	}
}

template <typename fpType>
void adjustSynapses(std::unique_ptr <fpType> &synapsesA, const std::unique_ptr <fpType> &synapsesB, const unsigned int &s_size, const unsigned int &index){
    // hebb rule for synapses (not original one)
    synapsesA[index] += (synapsesA[index]*synapsesB[index])/s_size;
}

template <typename neuronType>
void mergeChromosomes(std::unique_ptr<Brain<neuronType>> &b1, const std::unique_ptr<Brain<neuronType>> &b2, const std::unique_ptr<Brain<neuronType>> &b3, const unsigned int &breakingPoint){
	unsigned int n_size = b1->neurons.size();
	for(unsigned int i=0; i<breakingPoint; ++i){
		b1->chromosome[i] = b2->chromosome[i];
		if(b1->chromosome[i])
			adjustSynapses(b1->neurons[i/n_size]->synapses, b2->neurons[i/n_size]->synapses, b1->neurons[0]->s_size, i%n_size);
	}
	for(unsigned int i=breakingPoint; i<b1->chromosome.size(); ++i){
		b1->chromosome[i] = b3->chromosome[i];
		if(b1->chromosome[i])
			adjustSynapses(b1->neurons[i/n_size]->synapses, b3->neurons[i/n_size]->synapses, b1->neurons[0]->s_size, i%n_size);
	}
}

void mutation(std::vector <bool> &genome, const double &chance, std::uniform_real_distribution <> &distZeroOne, std::mt19937_64 &gen){
	// chance - for example 0.001 or 0.04
	if(distZeroOne(gen) <= chance){
		std::uniform_int_distribution <> distGenome(0, genome.size()-1);
		unsigned int k = distGenome(gen);
		genome[k] = !genome[k];
	}
}

template <typename neuronType>
void reproduce(std::vector <std::unique_ptr< Creature <neuronType>>> &population, double &bestFitness, std::uniform_int_distribution <> &distChrom, std::uniform_real_distribution <> &distZeroOne, std::mt19937_64 &gen){
    unsigned int k = 1;
	double mutationChance = 0.01;
    sort(population.begin(), population.end(), [](std::unique_ptr< Creature <neuronType>> &c1, std::unique_ptr< Creature <neuronType>> &c2)->bool{
        return c1->fitness > c2->fitness;
    });
    bestFitness = population[0]->fitness;

    while(k < population.size() && population[k-1]->fitness == population[k]->fitness){
        ++k;
    }
    if(bestFitness == 0)
        mutationChance = 0.7;
	if(k == population.size())
		return;
    for(unsigned int i=k; i<population.size(); ++i){
        mergeChromosomes(population[i]->brain, population[distChrom(gen) % k]->brain, population[distChrom(gen) % k]->brain, distChrom(gen));
		mutation(population[i]->brain->chromosome, mutationChance, distZeroOne, gen);
	}
}

#endif