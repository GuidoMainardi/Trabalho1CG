//
//  Poligono.hpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 18/08/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//

#ifndef Poligono_hpp
#define Poligono_hpp

#include <iostream>
using namespace std;


#ifdef WIN32
#include <windows.h>
#include <glut.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include <vector>

class Poligono
{
    vector <Ponto> Vertices;
public:
    Poligono();
    Ponto getVertice(int);
    unsigned long getNVertices();
    void insereVertice(Ponto);
    void desenhaPoligono();
    void desenhaVertices();
    void imprime();
};

#endif 
