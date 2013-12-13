/* AdjacencyGraph.cc - Adjacency graph data structure
 * Copyright 2013 Jamile Gonçalves
 *
 * This file is part of DoubleCutAndJoin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ADJACENCYGRAPH_H
#define ADJACENCYGRAPH_H
#include "Genome.h"
#include <iostream>
#include <limits.h>
#include <utility>
#include <queue>
#include <map>
#include <vector>
#include <set>

#define END_OF_TABLE INT_MAX

class Adjacency
{
public:
    int first;
    int second;
    std::vector <int> label;
    bool visited;

    bool isAdjacency();
    bool isTelomere();

    /* returns this \ {x}  */
    int setMinus(int x);
private:
    bool circularSingleton;

/*
    bool equals(Adjacency& other);
    */
};

typedef enum{undef, genomeA, genomeB} WhichGenome;

typedef struct
{
    unsigned int head;
    unsigned int tail;
} Location;

typedef struct
{
    unsigned int positionLabel;
} LocationLabel;

typedef struct
{
    WhichGenome startsIn;
    int start;
    int idxLast;
} Path;

class AdjacencyGraph
{
    public:
    AdjacencyGraph(Genome *a, Genome *b);
    ~AdjacencyGraph();

    private:

    Adjacency *adjA, *adjB;
    Location *locA, *locB;
    LocationLabel *locLabelA, *locLabelB;
    int idxEndOfAdjA, idxEndOfAdjB;
    int numLabels;

    
    std::deque< std::pair<WhichGenome,int> > cycles;
    std::deque<Path> oddPaths,evenPathsFromA, evenPathsFromB, adjacencies;

    /**
     * Constroi tabelas: Adjacency e Location.
     * @returns Número de adjacências
     */
    int constructTables(Genome *g,  std::set<int> *labels, Adjacency *&adj,
            Location *&loc, LocationLabel *&locLabel);

    /**
     * Armazena:
     * Primeiro elemento de cada caminho ímpar na fila: oddpaths
     * Primeiro elemento de cada caminho par na fila: evenpaths
     */
    void paths();

    int getLengthFromA(int i, int *idxLast = NULL);

    int getLengthFromB(int i, int *idxLast = NULL);

    int substPotentialInPaths(std::deque<Path> paths);

    int substPotentialInCycles(std::deque< std::pair<WhichGenome,int> > cycle);

    int substPotential();

    void findLabels(Genome *a, Genome *b);

    int totalAdjacencies(Genome *g, std::set<int> *labels);

    int DCJsubstDistance(Genome *a);

    Genome *a;
    Genome *b;
    std::set<int> labels;
    std::set<int> *labelsInA, *labelsInB;
    // Armazena a posição do Singleton Circular
    std::vector <int> circularSingleton;
    std::vector <int> linearSingleton;

    int n;
};

#endif
