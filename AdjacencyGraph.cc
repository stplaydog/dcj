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
#include "AdjacencyGraph.h"
#include <stack>
#include <map>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <set>
#include <bitset>

AdjacencyGraph::AdjacencyGraph(Genome *a, Genome *b)
{
    // Braga, Machado, Ribeiro e Stoye. Genomic distance
    // under gene substitutions. 2011.

    n = a->numGenes();

    adjA = adjB = NULL;

    findLabels(a, b);
    
    idxEndOfAdjA = constructTables(a, labelsInA, adjA, locA, locLabelA)+1;
    idxEndOfAdjB = constructTables(b, labelsInB, adjB, locB, locLabelB)+1;

    std::cout<< "Distância: "<< DCJsubstDistance(a);
    std::cout<< "\n ";
}

AdjacencyGraph::~AdjacencyGraph()
{
    delete adjA, adjB, locA, locB, locLabelA, locLabelB;
    if (labelsInA != NULL) delete labelsInA;
    if (labelsInB != NULL) delete labelsInB;
}

/**
 * Constroi tabelas: Adjacency e Location.
 * @returns Número de adjacências
 */
int AdjacencyGraph::constructTables(Genome *g, std::set<int> *labels,
        Adjacency *&adj,
        Location *&loc, LocationLabel *&locLabel)
{
    int n = g->numGenes();
    int adjacencyTableSize = 3*n + 2;

    adj = new Adjacency[adjacencyTableSize];
    loc = new Location[adjacencyTableSize];
    locLabel = new LocationLabel[adjacencyTableSize];
    int offset = 0;
    int numAdj = totalAdjacencies(g, labels);

    memset(locLabel, 0, adjacencyTableSize*sizeof(LocationLabel));

    std::vector<Chromosome*>::iterator cIterator;

    int k = 0;

    for(cIterator = g->chromosomes.begin();
            cIterator != g->chromosomes.end(); ++cIterator)
    {
        Chromosome *chr = *cIterator;

        int l = labels[k].size();

        int x = 1;

        if(chr->isLinear() == true)
        {
            // Caso o cromossomo já comece com label
            while(labels[k].begin() != labels[k].end() &&
                    abs(*labels[k].begin()) == abs((*chr)[x]))
            {
                adj[offset+1].label.push_back((*chr)[x]);
                labels[k].erase((*chr)[x]);
                ++x;
            }

            if(x != chr->length()+1)
            {
                adj[offset+1].first = (*chr)[x];
                adj[offset+1].second = 0;

                x = 1;

                for(int j = x+1; j <= chr->numAdjacencies(chr, l); ++j)
                {
                    adj[offset + j].first = -(*chr)[x];
                    ++x;

                    while(labels[k].begin() != labels[k].end() &&
                            abs(*labels[k].begin()) == abs((*chr)[x]))
                    {
                        adj[offset + j].label.push_back((*chr)[x]);
                        labels[k].erase((*chr)[x]);
                        ++x;
                    }

                    adj[offset + j].second = (*chr)[x];
                } // End for
            } // End if is not Singleton
            else // if is LinearSingleton
            {
                adj[offset + 1].first = 0;
                adj[offset + 1].second = 0;
                linearSingleton.push_back(offset+1);
            }
        } // End if is Linear
        else // if it's a circular chromosome
        {
            if(chr->length() == 1)
            {
                if(!labels[k].empty())
                {
                    adj[offset + 1].first = 0;
                    adj[offset + 1].second = 0;
                    adj[offset + 1].label.push_back((*chr)[x]);
                    labels[k].erase((*chr)[x]);
                    circularSingleton.push_back(offset+1); // Armazena a posição
                                                      // do singleton circular
                    ++x;
                }
                else
                {
                    adj[offset + 1].first = (*chr)[1];
                    adj[offset + 1].second = -(*chr)[1];
                }
            }
            else
            {
                bool endLabel = false;
                // Caso o cromossomo já comece com label
                while(labels[k].begin() != labels[k].end() &&
                        abs(*labels[k].begin()) == abs((*chr)[x]))
                {
                    adj[offset+1].label.push_back((*chr)[x]);
                    labels[k].erase((*chr)[x]);
                    ++x;
                }

                if(x != chr->length()+1)
                {
                    adj[offset + 1].first = (*chr)[1];
                    // Caso que termine com label
                    while(labels[k].begin() != labels[k].end() &&
                            abs(*labels[k].begin()) == abs((*chr)[chr->length()]))
                    {
                        adj[offset+1].label.push_back((*chr)[chr->length()]);
                        labels[k].erase((*chr)[chr->length()]);
                        endLabel = true;
                        ++x;
                    }
                    if(endLabel == true)
                        adj[offset + 1].second = - (*chr)[x];
                    else
                        adj[offset + 1].second = - (*chr)[chr->length()];

                    x = 1;

                    for(int j = x+1; j <= chr->numAdjacencies(chr, l); ++j)
                    {
                        adj[offset + j].first = - (*chr)[x];
                        ++x;

                        while(labels[k].begin() != labels[k].end() &&
                                abs(*labels[k].begin()) == abs((*chr)[x]))
                        {
                            adj[offset + j].label.push_back((*chr)[x]);
                            labels[k].erase((*chr)[x]);
                            ++x;
                        }
                        
                        adj[offset + j].second = (*chr)[x];
                    }
                }
                else // if is a circular singleton
                {
                    adj[offset + 1].first = 0;
                    adj[offset + 1].second = 0;
                    circularSingleton.push_back(offset+1);
                }
            }
        } // end else is circular

        for (int i = numAdj+1; i < adjacencyTableSize; ++i)
            adj[i].first = END_OF_TABLE;

        // Constroi tabela de localização
        for(int i = 1; adj[i].first != END_OF_TABLE; ++i)
        {
            if(adj[i].first > 0)
                loc[adj[i].first].tail = i;
            else
                loc[-adj[i].first].head = i;

            if(adj[i].second > 0)
                loc[adj[i].second].tail = i;
            else if(adj[i].second < 0)
                loc[-adj[i].second].head = i;

            if(!adj[i].label.empty())
            {
                for(std::vector<int>::iterator it = adj[i].label.begin();
                        it != adj[i].label.end(); it++)
                {
                    locLabel[abs(*it)].positionLabel = i;
                }
            }
        }
        ++k;
        offset = offset + chr->numAdjacencies(chr,l);
    } // end for each cromosome

    // Print
    std::cout<< "First: ";
    for(int i = 1; i <= 7; ++i)
        std::cout<< adj[i].first << ",";
    std::cout<< "\n";


    std::cout<< "Second: ";
    for(int i = 1; i <= 7; ++i)
        std::cout<< adj[i].second << ",";
    std::cout<< "\n";

    std::cout<< "Labels: ";
    for(int i = 1; i <= 7; ++i)
    {
        for (std::vector<int>::iterator it = adj[i].label.begin(); it != adj[i].label.end(); it++)
        {
            std::cout << *it << ",";
            // valor na posição apontada por it
        }
    }
    std::cout<< "\n";

    std::cout<< "Head: ";
    for(int i = 1; i <= 5; ++i)
        std::cout<< loc[i].head << ",";

    std::cout<< "\n";

    std::cout<< "tail: ";
    for(int i = 1; i <= 5; ++i)
        std::cout<< loc[i].tail << ",";
    std::cout<< "\n";

    std::cout<< "LocLabels: ";
    
    for (int i=19; i <= 27; ++i)
    {
        if(locLabel[i].positionLabel != 0)
            std::cout << locLabel[i].positionLabel << ",";
    }
    
    std::cout<< "\n";
    std::cout<< "\n";

}


bool Adjacency::isAdjacency()
{
    if (first == 0)
        return false;
    if(second != 0)
        return true;
    else
        return false;
}

/**
 * Armazena:
 * Primeiro elemento de cada caminho ímpar na fila: oddpaths
 * Primeiro elemento de cada caminho par na fila: evenpaths
 */
void AdjacencyGraph::paths()
{
    for(int i=1; adjA[i].first != END_OF_TABLE; ++i)
        adjA[i].visited = false;

    for(int i=1; adjB[i].first != END_OF_TABLE; ++i)
        adjB[i].visited = false;

    // Para cada telomero do Genoma A
    for(int i=1; adjA[i].first != END_OF_TABLE; ++i)
    {
        if( adjA[i].isTelomere() && !adjA[i].visited )
        {
            int idxLast;
            int length = getLengthFromA(i, &idxLast);

            Path path = { genomeA, adjA[i].first, idxLast };

            //adjA[idxLast].visited = true;

            if( length%2 == 0 )
                evenPathsFromA.push_back(path);
            else {
                if(length == 1)
                    adjacencies.push_back(path);
                else
                    oddPaths.push_back(path);
            }
        }
    }


    // Para cada telomero do Genoma B
    for(int i=1; adjB[i].first != END_OF_TABLE; ++i)
    {
        if( adjB[i].isTelomere() && !adjB[i].visited )
        {
            int idxLast;
            int length = getLengthFromB(i, &idxLast);

            Path path = { genomeB, adjB[i].first, idxLast};
            //adjB[idxLast].visited = true;

            if( length%2 == 0 )
                evenPathsFromB.push_back(path);
            else
            {
                std::cerr << "Error: invalid adjacency table!" << std::endl;
                throw "Error: invalid adjacency table!";
            }

        }
    }

    // Se não é telomero
    for(int i=1; adjB[i].first != END_OF_TABLE; ++i)
    {
        if( !adjB[i].isTelomere() && !adjB[i].visited )
        {
            int length = getLengthFromB(i);
            std::pair<WhichGenome,int> p(genomeB, adjB[i].first);
            cycles.push_back(p);
        }
    }

}


#define swapCond(a,b) if (a != 0 && b != 0) { temp = a; a = b; b = temp; }

int AdjacencyGraph::getLengthFromA(int i, int *idxLast)
{
    int length = 0;
    int j, k;
    int temp;

    if (idxLast != NULL) *idxLast = 0;

    do
    {
        adjA[i].visited = true;

        if(adjA[i].first > 0)
            j = locB[adjA[i].first].tail;
        else
            j = locB[-adjA[i].first].head;

        adjB[j].visited = true;

        length++;

        if(adjB[j].second == 0)
        {
            if (idxLast != NULL) *idxLast = j;
            break;
        }

        if(adjA[i].first == adjB[j].first)
        {
            if(adjB[j].second > 0)
                i = locA[adjB[j].second].tail;
            else
                i = locA[-adjB[j].second].head;

            if(adjB[j].second == adjA[i].first)
                swapCond(adjA[i].first, adjA[i].second);
        }
        else
        {
            if(adjB[j].first > 0)
                i = locA[adjB[j].first].tail;
            else
                i = locA[-adjB[j].first].head;

            if(adjB[j].first == adjA[i].first)
                swapCond(adjA[i].first, adjA[i].second);
        }

        length++;
    } while (adjA[i].second != 0 && !adjA[i].visited);

    if (idxLast != NULL && length % 2 == 0)
    {
        adjA[i].visited = true;
        *idxLast = i;
    }

    return length;
}

int AdjacencyGraph::getLengthFromB(int i, int *idxLast)
{
    int length = 0;
    int j, k;
    int temp;

    if (idxLast != NULL) *idxLast = 0;

    do
    {
        adjB[i].visited = true;

        if(adjB[i].first > 0)
            j = locA[adjB[i].first].tail;
        else
            j = locA[-adjB[i].first].head;

        adjA[j].visited = true;

        length++;

        if(adjA[j].second == 0)
        {
            if (idxLast != NULL) *idxLast = j;
            break;
        }

        if(adjB[i].first == adjA[j].first)
        {
            if(adjA[j].second > 0)
                i = locB[adjA[j].second].tail;
            else
                i = locB[-adjA[j].second].head;

            if(adjA[j].second == adjB[i].first)
                swapCond(adjB[i].first, adjB[i].second);
        }
        else
        {
            if(adjA[j].first > 0)
                i = locB[adjA[j].first].tail;
            else
                i = locB[-adjA[j].first].head;

            if(adjA[j].first == adjB[i].first)
                swapCond(adjB[i].first, adjB[i].second);
        }

        length++;
    } while (adjB[i].second != 0 && !adjB[i].visited);

    if (idxLast != NULL && length % 2 == 0) {
        adjB[i].visited = true;
        *idxLast = i;
    }

    return length;
}

int AdjacencyGraph::DCJsubstDistance(Genome *a)
{
    paths();

    int g = a->numGenes() - numLabels;
    int pL = linearSingleton.size();
    int pC = circularSingleton.size();
    int b = oddPaths.size() + adjacencies.size();
    int c = cycles.size();
    int pathTable[128] = { 0 };
    int sigma = substPotential(pathTable);

    int u = getU(pathTable); // arrumar
    int v = getV(pathTable);
    int w = getW(pathTable); // arrumar
    int x = getX(pathTable); // arrumar
    int y = getY(pathTable);
    int z = getZ(pathTable);

    int d = g - c - (b/2) + sigma - pL - pC - 2*u - 3*v - 2*w - x - 2*y - z;

    return d;
}

/*
 * Determinar o número total de runs
 */

int AdjacencyGraph::substPotential(int pathTable[128])
{
    int oddPath = 1;
    int evenPath = 2;
    int numPaths = oddPaths.size()+evenPathsFromA.size()+evenPathsFromB.size();
    
    int count = 0;

    int c = substPotentialInCycles(cycles);
    int odd = substPotentialInPaths(oddPaths, oddPath, pathTable, &count);
    int evenFromA = substPotentialInPaths(evenPathsFromA, evenPath, pathTable, &count);
    int evenFromB = substPotentialInPaths(evenPathsFromB, evenPath, pathTable, &count);

    int substPotential = c + odd + evenFromA + evenFromB;

    for(byte i = 0; i < 128; ++i) {
        if (pathToStr(i) != NULL )
            printPath(i, std::cout) << " = " << (int)i << "\t" << pathTable[i] << std::endl;
    }

    return substPotential;

}

int AdjacencyGraph::getU(int *pathTable) {
    int u = 0;
    while (pathTable[AAab4] > 0 && pathTable[BBab4] > 0) {
        ++u;
        --pathTable[AAab4];
        --pathTable[BBab4];
    }
    return u;
}

int AdjacencyGraph::getV(int *pathTable) {
    int v = 0;
    while (pathTable[AAab4] > 1 && pathTable[BBa1] > 0 && pathTable[BBb1] > 0) {
        ++v;
        --pathTable[AAab4];
        --pathTable[AAab4];
        --pathTable[BBa1];
        --pathTable[BBb1];
    }
    while (pathTable[BBab4] > 1 && pathTable[AAa1] > 0 && pathTable[AAb1] > 0) {
        ++v;
        --pathTable[BBab4];
        --pathTable[BBab4];
        --pathTable[AAa1];
        --pathTable[AAb1];
    }
    return v;
}

int AdjacencyGraph::getW(int *pathTable) {
    int w = 0, oldw = 0;
    do {
        oldw = w;
        while (pathTable[AAab4] > 0 && pathTable[BBa1] > 0 &&
                pathTable[ABab4] > 0) {
            ++w;
            --pathTable[AAab4];
            --pathTable[BBa1];
            --pathTable[ABab4];
        }
        while (pathTable[AAab4] > 0 && pathTable[BBb1] > 0 &&
                pathTable[ABba4] > 0) {
            ++w;
            --pathTable[AAab4];
            --pathTable[BBb1];
            --pathTable[ABba4];
        }
        while (pathTable[BBab4] > 0 && pathTable[AAa1] > 0 &&
                pathTable[ABba4] > 0) {
            ++w;
            --pathTable[BBab4];
            --pathTable[AAa1];
            --pathTable[ABba4];
        }
        while (pathTable[BBab4] > 0 && pathTable[AAb1] > 0 &&
                pathTable[ABab4] > 0) {
            ++w;
            --pathTable[BBab4];
            --pathTable[AAb1];
            --pathTable[ABab4];
        }
        while (pathTable[AAab4] > 1 && pathTable[BBa1] > 0) {
            ++w;
            ++pathTable[AAb3];
            --pathTable[AAab4];
            --pathTable[AAab4];
            --pathTable[BBa1];
        }
        while (pathTable[AAab4] > 1 && pathTable[BBb1] > 0) {
            ++w;
            ++pathTable[AAa3];
            --pathTable[AAab4];
            --pathTable[AAab4];
            --pathTable[BBb1];
        }
        while (pathTable[BBab4] > 1 && pathTable[AAa1] > 0) {
            ++w;
            ++pathTable[BBb3];
            --pathTable[BBab4];
            --pathTable[BBab4];
            --pathTable[AAa1];
        }
        while (pathTable[BBab4] > 1 && pathTable[AAb1] > 0) {
            ++w;
            ++pathTable[BBa3];
            --pathTable[BBab4];
            --pathTable[BBab4];
            --pathTable[AAb1];
        }
    } while (oldw != w);
    return w;
}

int AdjacencyGraph::getX(int *pathTable) {
    int x = 0, oldx = 0;
    do {
        oldx = x;
        while (pathTable[AAab4] > 1) {
            ++x;
            ++pathTable[AAa3];
            ++pathTable[AAb3];
            --pathTable[AAab4];
            --pathTable[AAab4];
        }
        while (pathTable[BBab4] > 1) {
            ++x;
            ++pathTable[BBa3];
            ++pathTable[BBb3];
            --pathTable[BBab4];
            --pathTable[BBab4];
        }
        while (pathTable[AAab4] > 0 && pathTable[ABab4]) {
            ++x;
            ++pathTable[AAa3];
            --pathTable[AAab4];
            --pathTable[ABab4];
        }
        while (pathTable[AAab4] > 0 && pathTable[ABba4] > 0) {
            ++x;
            ++pathTable[AAb3];
            --pathTable[AAab4];
            --pathTable[ABba4];
        }
        while (pathTable[BBab4] > 0 && pathTable[ABab4] > 0) {
            ++x;
            ++pathTable[BBb3];
            --pathTable[BBab4];
            --pathTable[ABab4];
        }
        while (pathTable[BBab4] > 0 && pathTable[ABba4] > 0) {
            ++x;
            ++pathTable[BBa3];
            --pathTable[BBab4];
            --pathTable[ABba4];
        }
        while (pathTable[AAa1] > 0 && pathTable[BBab4] > 0) {
            ++x;
            ++pathTable[ABab4];
            --pathTable[AAa1];
            --pathTable[BBab4];
        }
        while (pathTable[AAb1] > 0 && pathTable[BBab4] > 0) {
            ++x;
            ++pathTable[ABba4];
            --pathTable[AAb1];
            --pathTable[BBab4];
        }
        while (pathTable[AAab4] > 0 && pathTable[BBa1] > 0) {
            ++x;
            ++pathTable[ABba4];
            --pathTable[AAab4];
            --pathTable[BBa1];
        }
        while (pathTable[AAab4] > 0 && pathTable[BBb1] > 0) {
            ++x;
            ++pathTable[ABab4];
            --pathTable[AAab4];
            --pathTable[BBb1];
        }
        while (pathTable[AAab2] > 0 && pathTable[BBab4] > 0) {
            ++x;
            --pathTable[AAab2];
            --pathTable[BBab4];
        }
        while (pathTable[AAab4] > 0 && pathTable[BBab2] > 0) {
            ++x;
            --pathTable[AAab4];
            --pathTable[BBab2];
        }
        while (pathTable[AAab2] > 0 && pathTable[BBab2] > 0) {
            ++x;
            --pathTable[AAab2];
            --pathTable[BBab2];
        }
        while (pathTable[AAa3] > 0 && pathTable[BBab4] > 0) {
            ++x;
            --pathTable[AAa3];
            --pathTable[BBab4];
        }
        while (pathTable[AAb3] > 0 && pathTable[BBab4] > 0) {
            ++x;
            --pathTable[AAb3];
            --pathTable[BBab4];
        }
        while (pathTable[AAab4] > 0 && pathTable[BBa3] > 0) {
            ++x;
            --pathTable[AAab4];
            --pathTable[BBa3];
        }
        while (pathTable[AAab4] > 0 && pathTable[BBb3] > 0) {
            ++x;
            --pathTable[AAab4];
            --pathTable[BBb3];
        }
        while (pathTable[AAa1] > 0 && pathTable[BBa1] > 0) {
            ++x;
            --pathTable[AAa1];
            --pathTable[BBa1];
        }
        while (pathTable[AAb1] > 0 && pathTable[BBb1] > 0) {
            ++x;
            --pathTable[AAb1];
            --pathTable[BBb1];
        }
        while (pathTable[AAa1] > 0 && pathTable[BBab2] > 0) {
            ++x;
            --pathTable[AAa1];
            --pathTable[BBab2];
        }
        while (pathTable[AAb1] > 0 && pathTable[BBab2] > 0) {
            ++x;
            --pathTable[AAb1];
            --pathTable[BBab2];
        }
        while (pathTable[AAab2] > 0 && pathTable[BBa1] > 0) {
            ++x;
            --pathTable[AAab2];
            --pathTable[BBa1];
        }
        while (pathTable[AAab2] > 0 && pathTable[BBb1] > 0) {
            ++x;
            --pathTable[AAab2];
            --pathTable[BBb1];
        }
        while (pathTable[AAa1] > 0 && pathTable[BBa3] > 0) {
            ++x;
            --pathTable[AAa1];
            --pathTable[BBa3];
        }
        while (pathTable[AAb1] > 0 && pathTable[BBb3] > 0) {
            ++x;
            --pathTable[AAb1];
            --pathTable[BBb3];
        }
        while (pathTable[AAa3] > 0 && pathTable[BBa1] > 0) {
            ++x;
            --pathTable[AAa3];
            --pathTable[BBa1];
        }
        while (pathTable[AAb3] > 0 && pathTable[BBb1] > 0) {
            ++x;
            --pathTable[AAb3];
            --pathTable[BBb1];
        }
        while (pathTable[ABab4] > 0 && pathTable[ABba4] > 0) {
            ++x;
            --pathTable[ABab4];
            --pathTable[ABba4];
        }
    } while (oldx != x);
    return x;
}

int AdjacencyGraph::getY(int *pathTable) {
    int y = 0;
    while (pathTable[ABab4] > 1 && pathTable[AAb1] > 0 && pathTable[BBa1] > 0) {
        ++y;
        --pathTable[ABab4];
        --pathTable[ABab4];
        --pathTable[AAb1];
        --pathTable[BBa1];
    }
    while (pathTable[ABba4] > 1 && pathTable[AAa1] > 0 && pathTable[BBb1] > 0) {
        ++y;
        --pathTable[ABba4];
        --pathTable[ABba4];
        --pathTable[AAa1];
        --pathTable[BBb1];
    }
    return y;
}


int AdjacencyGraph::getZ(int *pathTable) {
    int z = 0, oldz = 0;
    do {
        oldz = z;
        while (pathTable[ABab4] > 0 && pathTable[AAab2] > 0 &&
                pathTable[BBa3] > 0) {
            ++z;
            --pathTable[ABab4];
            --pathTable[AAab2];
            --pathTable[BBa3];
        }
        while (pathTable[ABba4] > 0 && pathTable[AAab2] > 0 &&
                pathTable[BBb3] > 0) {
            ++z;
            --pathTable[ABba4];
            --pathTable[AAab2];
            --pathTable[BBb3];
        }
        while (pathTable[ABba4] > 0 && pathTable[AAa3] > 0 &&
                pathTable[BBab2] > 0) {
            ++z;
            --pathTable[ABba4];
            --pathTable[AAa3];
            --pathTable[BBab2];
        }
        while (pathTable[ABab4] > 0 && pathTable[AAb3] > 0 &&
                pathTable[BBab2] > 0) {
            ++z;
            --pathTable[ABab4];
            --pathTable[AAb3];
            --pathTable[BBab2];
        }
        while (pathTable[ABab4] > 0 && pathTable[AAb1] > 0 &&
                pathTable[BBa3] > 0) {
            ++z;
            --pathTable[ABab4];
            --pathTable[AAb1];
            --pathTable[BBa3];
        }
        while (pathTable[ABab4] > 0 && pathTable[AAb3] > 0 &&
                pathTable[BBa1] > 0) {
            ++z;
            --pathTable[ABab4];
            --pathTable[AAb3];
            --pathTable[BBa1];
        }
        while (pathTable[ABba4] > 0 && pathTable[AAa1] > 0 &&
                pathTable[BBb3] > 0) {
            ++z;
            --pathTable[ABba4];
            --pathTable[AAa1];
            --pathTable[BBb3];
        }
        while (pathTable[ABba4] > 0 && pathTable[AAa3] > 0 &&
                pathTable[BBb1] > 0) {
            ++z;
            --pathTable[ABba4];
            --pathTable[AAa3];
            --pathTable[BBb1];
        }
        while (pathTable[ABab4] > 0 && pathTable[AAb1] > 0 &&
                pathTable[BBa1] > 0) {
            ++z;
            ++pathTable[ABba4];
            --pathTable[ABab4];
            --pathTable[AAb1];
            --pathTable[BBa1];
        }
        while (pathTable[ABba4] > 0 && pathTable[AAa1] > 0 &&
                pathTable[BBb1] > 0) {
            ++z;
            ++pathTable[ABab4];
            --pathTable[ABba4];
            --pathTable[AAa1];
            --pathTable[BBb1];
        }
        while (pathTable[ABab4] > 1 && pathTable[AAb1] > 0) {
            ++z;
            ++pathTable[AAa3];
            --pathTable[ABab4];
            --pathTable[ABab4];
            --pathTable[AAb1];
        }
        while (pathTable[ABab4] > 1 && pathTable[BBa1] > 0) {
            ++z;
            ++pathTable[BBb3];
            --pathTable[ABab4];
            --pathTable[ABab4];
            --pathTable[BBa1];
        }
        while (pathTable[ABba4] > 1 && pathTable[AAa1] > 0) {
            ++z;
            ++pathTable[AAb3];
            --pathTable[ABba4];
            --pathTable[ABba4];
            --pathTable[AAa1];
        }
        while (pathTable[ABba4] > 1 && pathTable[BBb1] > 0) {
            ++z;
            ++pathTable[BBa3];
            --pathTable[ABba4];
            --pathTable[ABba4];
            --pathTable[BBb1];
        }
    } while (oldz != z);
    return z;
}

int AdjacencyGraph::getK(int numRuns) {
    if (numRuns == 0)
    {
    } else if (numRuns % 4 == 0) {
        return 4;
    } else {
        return numRuns % 4;
    }
}

// k = 0 means epsilon
byte AdjacencyGraph::pathToByte(WhichGenome firstElementIn,
                WhichGenome lastElementIn,
                WhichGenome firstRunIn,
                WhichGenome lastRunIn, int k) {
  byte ret = 0;

  if (firstElementIn == genomeB && lastElementIn == genomeA) {
     // replaces BA?? for AB??
    firstElementIn = genomeA;
    lastElementIn = genomeB;
    if (firstRunIn == genomeA && lastRunIn == genomeB) {
      firstRunIn = genomeB;
      lastRunIn == genomeA;
    } else if (firstRunIn == genomeB && lastRunIn == genomeA) {
      firstRunIn = genomeA;
      lastRunIn == genomeB;
    }
  } else if (firstElementIn == lastElementIn) {
    // replaces AAba+2, AAba+4, BBba+2, BBba+4 for
    //          AAab+2, AAab+4, BBab+2, BBab+4
    if (firstRunIn == genomeB && lastRunIn == genomeA) {
      firstRunIn = genomeA;
      lastRunIn = genomeB;
    }
  }

  if (firstElementIn == genomeB) {
    ret |= 64; // 64 = 1000000b
  }
  if (lastElementIn == genomeB) {
    ret |= 32; // 32 = 100000b
  }
  if (k > 0) {
    if (firstRunIn == genomeB) {
      ret |= 16; // 16 = 10000b
    }
    if (k%2 == 0) {
      if (lastRunIn == genomeB) {
        ret |= 8; // 16 = 1000b
      }
    }
    ret |= k & 7 ; //  K & 111b == k2 k1 k0
  }
  return ret;
}

const char *AdjacencyGraph::pathToStr(byte representation) {
    static const
    char *strByte[] = { "AAe"   , "AAa+1" , NULL     , "AAa+3" , NULL     ,
                        NULL    , NULL    , NULL     , NULL    , NULL     ,
             /* 10 */   "AAab+2", NULL    , "AAab+4" , NULL    , NULL     ,
                        NULL    , NULL    , "AAb+1"  , NULL    , "AAb+3"  ,
             /* 20 */   NULL    , NULL    , NULL     , NULL    , NULL     ,
                        NULL    , NULL    , NULL     , NULL    , NULL     ,
             /* 30 */   NULL    , NULL    , "ABe"    , "ABa+1" , NULL     ,
                        "ABa+3" , NULL    , NULL     , NULL    , NULL     ,
             /* 40 */   NULL    , NULL    , "ABab+2" , NULL    , "ABab+4" ,
                        NULL    , NULL    , NULL     , NULL    , "ABb+1"  ,
             /* 50 */   "ABba+2", "ABb+3" , "ABba+4" , NULL    , NULL     ,
                        NULL    , NULL    , NULL     , NULL    , NULL     ,
             /* 60 */   NULL    , NULL    , NULL     , NULL    , NULL     ,
                        NULL    , NULL    , NULL     , NULL    , NULL     ,
             /* 70 */   NULL    , NULL    , NULL     , NULL    , NULL     ,
                        NULL    , NULL    , NULL     , NULL    , NULL     ,
             /* 80 */   NULL    , NULL    , NULL     , NULL    , NULL     ,
                        NULL    , NULL    , NULL     , NULL    , NULL     ,
             /* 90 */   NULL    , NULL    , NULL     , NULL    , NULL     ,
                        NULL    , "BBe"   , "BBa+1"  , NULL    , "BBa+3"  ,
             /*100 */   NULL    , NULL    , NULL     , NULL    , NULL     ,
                        NULL    , "BBab+2", NULL     , "BBab+4", NULL     ,
             /*110 */   NULL    , NULL    , NULL     , "BBb+1" , NULL     ,
                        "BBb+3" , NULL    , NULL     , NULL    , NULL     ,
             /*120 */   NULL    , NULL    , NULL     , NULL    , NULL     ,
                        NULL    , NULL    , NULL };
    return strByte[representation];
}

std::ostream &AdjacencyGraph::printPath(byte representation,
                                        std::ostream &os) {
      const char *str = pathToStr(representation);
      if (str != NULL)
          os << str;
      else
          os << "invalid";
      return os;
  }

/*
 * Determinar o número de runs nos ciclos
 */

int AdjacencyGraph::substPotentialInCycles(std::deque< std::pair<WhichGenome,int> > cycle)
{
    WhichGenome lastLabelIn = undef;
    WhichGenome whereThis = undef;

    int aRuns, bRuns, numRuns;
    int i, idx, temp;
    int first; // Condição de parada do ciclo

    Adjacency *adjTable;
    Location *locTable;

    int substPTotal = 0; // Somatório de potenciais de substituição
    int substP; // potencial de substituição da componente c

    std::deque< std::pair<WhichGenome,int> >::iterator it;

    for(it = cycle.begin(); it != cycle.end(); it++)
    {
        aRuns = 0;
        bRuns = 0;
        numRuns = 0;
        std::pair<WhichGenome,int> p = *it;

        i = p.second;

        if(p.first == genomeA)
        {
            whereThis = genomeA;
            adjTable = adjA;
            locTable = locA;
        }
        else if(p.first == genomeB)
        {
            whereThis = genomeB;
            adjTable = adjB;
            locTable = locB;
        }

        if(i > 0)
            idx = locTable[i].tail;
        else if(i < 0)
            idx = locTable[-i].head;

        first = adjTable[idx].setMinus(i);

        for(;;)
        {
            // A adjacencia possui label?
            if(!adjTable[idx].label.empty())
            {
                if(lastLabelIn != whereThis)
                {
                    if(whereThis == genomeA)
                        ++aRuns;
                    if(whereThis == genomeB)
                        ++bRuns;
                }
                lastLabelIn = whereThis;
            }

            // Condição de parada
            if(i == first)
                break;

            // Atualizando a adjacencia
            if(whereThis == genomeA)
            {
                whereThis = genomeB;
                adjTable = adjB;
                locTable = locB;
            }
            else if(whereThis == genomeB)
            {
                whereThis = genomeA;
                adjTable = adjA;
                locTable = locA;
            }

            if(i > 0)
                idx = locTable[i].tail;
            else if(i < 0)
                idx = locTable[-i].head;

            temp = i;

            i = adjTable[idx].setMinus(temp);
        }

        numRuns = aRuns + bRuns;

        if(numRuns >= 1)
            substP = ceil((numRuns+1.0)/4.0);
        else
            substP = 0;

        substPTotal = substPTotal + substP;
    }
    return substPTotal;
}

/*
 * Determinar o número de runs nos caminhos pares e ímpares
 */

int AdjacencyGraph::substPotentialInPaths(std::deque<Path> paths,
                                int parity, int *pathTable, int *count)
{
    WhichGenome lastLabelIn = undef;
    WhichGenome whereThis = undef;

    WhichGenome startsRunIn = undef;
    WhichGenome endsRunIn = undef;

    WhichGenome startsPathIn = undef;
    WhichGenome endsPathIn = undef;

    int k;

    int aRuns, bRuns, numRuns;
    int i, idx, temp;
    int first; // Condição de parada do ciclo
    
    Adjacency *adjTable;
    Location *locTable;
    
    int substPTotal = 0; // Somatório de potenciais de substituição
    int substP; // potencial de substituição da componente c

    std::deque<Path>::iterator it;

    // Determinar o número de runs

    for(it = paths.begin(); it != paths.end(); it++)
    {
        aRuns = 0;
        bRuns = 0;
        numRuns = 0;
        Path p = *it;

        i = p.start;

        if(parity == 1)
        {
            // Odd Path
            if(p.startsIn == genomeA)
            {
                startsPathIn = genomeA;
                endsPathIn = genomeB;
            }
            else if(p.startsIn == genomeB)
            {
                startsPathIn = genomeB;
                endsPathIn = genomeA;
            }
        }
        else if(parity == 2)
        {
            // Even Path
            if(p.startsIn == genomeA)
            {
                startsPathIn = genomeA;
                endsPathIn = genomeA;
            }
            else if(p.startsIn == genomeB)
            {
                startsPathIn = genomeB;
                endsPathIn = genomeB;
            }
        }

        if(p.startsIn == genomeA)
        {
            whereThis = genomeA;
            adjTable = adjA;
            locTable = locA;
        }
        else if(p.startsIn == genomeB)
        {
            whereThis = genomeB;
            locTable = locB;
        }

        if(i > 0)
            idx = locTable[i].tail;
        else if(i < 0)
            idx = locTable[-i].head;

        for(;;)
        {   
            // A adjacencia possui label?
            if(!adjTable[idx].label.empty())
            {
                if(startsRunIn == undef)
                    startsRunIn = whereThis;

                if(lastLabelIn != whereThis)
                {
                    if(whereThis == genomeA)
                        ++aRuns;
                    if(whereThis == genomeB)
                        ++bRuns;
                }
                lastLabelIn = whereThis;
            }

            // Condição de parada
            if(i == 0)
            {
                endsRunIn = lastLabelIn;
                break;
            }
            // Atualizando a adjacencia
            if(whereThis == genomeA)
            {
                whereThis = genomeB;
                adjTable = adjB;
                locTable = locB;
            }
            else if(whereThis == genomeB)
            {
                whereThis = genomeA;
                adjTable = adjA;
                locTable = locA;
            }

            if(i > 0)
                idx = locTable[i].tail;
            else if(i < 0)
                idx = locTable[-i].head;

            temp = i;

            i = adjTable[idx].setMinus(temp);
        }

        numRuns = aRuns + bRuns;

        if(numRuns >= 1)
            substP = ceil((numRuns+1.0)/4.0);
        else
            substP = 0;

        substPTotal = substPTotal + substP;

        k = getK(numRuns);

        byte type = pathToByte(startsPathIn, endsPathIn, startsRunIn, endsRunIn, k);
        pathTable[type]++;
    }
    return substPTotal;
}

bool Adjacency::isTelomere()
{
    if (first == 0)
        return false;
    if(second == 0)
        return true;
    else
        return false;
}



void AdjacencyGraph::findLabels(Genome *a, Genome *b)
{
    labels.clear();

    std::set<int> markersA, markersB;

    std::vector<Chromosome*>::iterator cIteratorA;
    std::vector<Chromosome*>::iterator cIteratorB;

    if (labelsInA != NULL) delete labelsInA;
    if (labelsInB != NULL) delete labelsInB;

    labelsInA = new std::set<int>[a->chromosomes.size()];
    labelsInB = new std::set<int>[b->chromosomes.size()];


    for(cIteratorA = a->chromosomes.begin();
            cIteratorA != a->chromosomes.end(); ++cIteratorA)
    {
        Chromosome *chrA = *cIteratorA;
        for(int i = 1; i <= chrA->length(); ++i)
            markersA.insert(abs((*chrA)[i]));
    }

    for(cIteratorB = b->chromosomes.begin();
            cIteratorB != b->chromosomes.end(); ++cIteratorB)
    {
        Chromosome *chrB = *cIteratorB;
        for(int i = 1; i <= chrB->length(); ++i)
            markersB.insert(abs((*chrB)[i]));
    }

    int k = 0;

    for(cIteratorA = a->chromosomes.begin();
            cIteratorA != a->chromosomes.end(); ++cIteratorA)
    {
        Chromosome *chrA = *cIteratorA;
        for(int i = 1; i <= chrA->length(); ++i)
        {
            if( markersB.find(abs((*chrA)[i])) == markersB.end() )
            {
                labelsInA[k].insert(abs((*chrA)[i]));
                numLabels++;
            }
        }
        ++k;
    }

    k = 0;

    for(cIteratorB = b->chromosomes.begin();
            cIteratorB != b->chromosomes.end(); ++cIteratorB)
    {
        Chromosome *chrB = *cIteratorB;
        for(int i = 1; i <= chrB->length(); ++i)
        {
            if( markersA.find(abs((*chrB)[i])) == markersA.end() )
            {
                labelsInB[k].insert(abs((*chrB)[i]));
            }
        }
        ++k;
    }
}

int AdjacencyGraph::totalAdjacencies(Genome *g, std::set<int> *labels)
{
    int totalAdj = 0;
    int k = 0;

    std::vector<Chromosome*>::iterator cIterator;

    for(cIterator = g->chromosomes.begin();
            cIterator != g->chromosomes.end(); ++cIterator)
    {
        Chromosome *chr = *cIterator;

        int n = chr->length();
        int l = labels[k].size();

        if(chr->isLinear() == true)
            totalAdj = totalAdj + n+1 - l;
        else
            totalAdj = totalAdj + n - l;
        
        if(n == l)
            totalAdj = totalAdj + 1;

        k++;
    }
    return totalAdj;
}

/*
bool Adjacency::equals(Adjacency &a)
{
    return((first == a.first)&&(second==a.second)
                            || (first==a.second)&&(second==a.first));
}
*/
int Adjacency::setMinus(int x)
{
    if(first == x)
        return second;
    else
        return first;
}