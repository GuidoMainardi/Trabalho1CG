//
//  Ponto.cpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 18/08/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//

#include "Ponto.h"
Ponto::Ponto ()
{
    x=y=z=0;
}
Ponto::Ponto(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}
void Ponto::set(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}
void Ponto::imprime() {
    cout << "(" << x << ", " << y << ")" << flush;
}
