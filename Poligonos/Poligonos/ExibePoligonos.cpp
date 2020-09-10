// **********************************************************************
// PUCRS/Escola Politcnica
// COMPUTAÌO GRçFICA
//
// Programa basico para criar aplicacoes 2D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.
//

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>
#include <fstream>


using namespace std;

#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include "Poligono.h"
#include "Temporizador.h"
Temporizador T;
double AccumDeltaT=0;

Poligono Mapa;
Poligono ConvexHull;
int Nfaixas = 11, NPontos = 200000;
float *faixas = new float[Nfaixas];
Ponto *pontos = new Ponto[NPontos];
int *cores = new int[NPontos];
vector<int> *arestas = new vector<int>[Nfaixas - 1];
// Limites lgicos da rea de desenho
Ponto Min, Max;

// **********************************************************************
//    Calcula o produto escalar entre os vetores V1 e V2
// **********************************************************************
double ProdEscalar(Ponto v1, Ponto v2)
{
    return v1.x*v2.x + v1.y*v2.y+ v1.z*v2.z;
}
// **********************************************************************
//    Calcula o produto vetorial entre os vetores V1 e V2
// **********************************************************************
void ProdVetorial (Ponto v1, Ponto v2, Ponto &vresult)
{
    vresult.x = v1.y * v2.z - (v1.z * v2.y);
    vresult.y = v1.z * v2.x - (v1.x * v2.z);
    vresult.z = v1.x * v2.y - (v1.y * v2.x);
}
/* ********************************************************************** */
/*                                                                        */
/*  Calcula a interseccao entre 2 retas (no plano "XY" Z = 0)             */
/*                                                                        */
/* k : ponto inicial da reta 1                                            */
/* l : ponto final da reta 1                                              */
/* m : ponto inicial da reta 2                                            */
/* n : ponto final da reta 2                                              */
/*                                                                        */
/* s: valor do parmetro no ponto de interseo (sobre a reta KL)         */
/* t: valor do parmetro no ponto de interseo (sobre a reta MN)         */
/*                                                                        */
/* ********************************************************************** */
int intersec2d(Ponto k, Ponto l, Ponto m, Ponto n, double &s, double &t)
{
    double det;

    det = (n.x - m.x) * (l.y - k.y)  -  (n.y - m.y) * (l.x - k.x);

    if (det == 0.0)
        return 0 ; // no h interseco

    s = ((n.x - m.x) * (m.y - k.y) - (n.y - m.y) * (m.x - k.x))/ det ;
    t = ((l.x - k.x) * (m.y - k.y) - (l.y - k.y) * (m.x - k.x))/ det ;

    return 1; // h interseco
}

int intersecLinha(Ponto k, Ponto l, Ponto m, Ponto n)
{
    double det;

    det = (n.x - m.x) * (l.y - k.y)  -  (n.y - m.y) * (l.x - k.x);

    if (det == 0.0)
        return 0 ; // no h interseco
    return 1; // h interseco
}
// **********************************************************************
bool HaInterseccao(Ponto k, Ponto l, Ponto m, Ponto n)
{
    int ret;
    double s,t;
    ret = intersec2d( k,  l,  m,  n, s, t);
    if (!ret) return false;
    if (s>=0.0 && s <=1.0 && t>=0.0 && t<=1.0)
        return true;
    else return false;
}
// **********************************************************************
Ponto vet(Ponto a, Ponto b){
    Ponto resp;
    resp.set(b.x - a.x, b.y - a.y, b.z - a.z);
    return resp;
}
// **********************************************************************
bool equals(Ponto a, Ponto b){
    if(a.x == b.x && a.y == b.y && a.z == b.z){
        return true;
    }
    return false;
}
// **********************************************************************
// **********************************************************************
Poligono ConvHull(Poligono P){
    Poligono resp;
    Ponto atual = P.getVertice(0);
    Ponto fim;
    for(int i = 0; i < P.getNVertices(); i++){
        if(P.getVertice(i).x < atual.x){
            atual = P.getVertice(i);
        }
    }

    do{
        resp.insereVertice(atual);
        fim = P.getVertice(0);
        for(int i = 0; i < P.getNVertices(); i++){
            Ponto temp;
            ProdVetorial(vet(atual,fim),vet(atual, P.getVertice(i)), temp);
            if(equals(fim, atual) || temp.z > 0){
                fim = P.getVertice(i);
            }
        }
        atual = fim;
    }while(!equals(fim,resp.getVertice(0)));
    return resp;
}
// **********************************************************************
bool arestaINfaixa(Ponto inicFaixa, Ponto fimFaixa, int * aresta){
    Ponto inicAresta = Mapa.getVertice(aresta[0]);
    Ponto fimAresta = Mapa.getVertice(aresta[1]);
    if(inicAresta.y < fimAresta.y){
        if(inicAresta.y > fimFaixa.y) {return false;}
        if(fimAresta.y < inicFaixa.y) {return false;}
    }
    if(inicAresta.y > fimAresta.y){
        if(fimAresta.y > fimFaixa.y) {return false;}
        if(inicAresta.y < inicFaixa.y) {return false;}
    }else{
        if(inicAresta.y <= fimFaixa.y && inicAresta.y >= inicFaixa.y) {return true;}
    }
    return true;
}
// **********************************************************************
void criaFaixas(){
    faixas[0] = Min.y;
    float dif = fabs(Max.y- Min.y)/ (Nfaixas - 1);
    for(int i = 1; i < Nfaixas; i ++){
        faixas[i] = faixas[i - 1] + dif;
    }
    for(int j = 0; j < Nfaixas - 1; j ++){
        Ponto inicFaixa, fimFaixa;
        inicFaixa.set(0, faixas[j], 0);
        fimFaixa.set(0, faixas[j + 1], 0);
        for(int i = 0; i < Mapa.getNVertices(); i ++){
            int temp[2] = {i, (i + 1) % Mapa.getNVertices()};
            if(arestaINfaixa(inicFaixa, fimFaixa, temp)){
                arestas[j].push_back(i);
            }
        }
    }
}
//***********************************************************************
bool dentroConvexo(Ponto p, Poligono Conv){
    for(int i = 0; i < Conv.getNVertices(); i ++){
        Ponto resp, aresta;
        aresta.set(Conv.getVertice((i + 1)%Conv.getNVertices()).x - Conv.getVertice(i).x,
                   Conv.getVertice((i + 1)%Conv.getNVertices()).y - Conv.getVertice(i).y,
                   0);
        p.set(p.x - Conv.getVertice(i).x,
              p.y - Conv.getVertice(i).y,
              0);
        ProdVetorial(aresta, p, resp);
        if(resp.z > 0){ return false;}
        p.set(p.x + Conv.getVertice(i).x,
              p.y + Conv.getVertice(i).y,
              0);
    }
    return true;
}
//***********************************************************************
bool maxOUmin(int i, Poligono p){
    Ponto atual, anterior, proximo;
    atual = p.getVertice(i);
    anterior = p.getVertice((i - 1) % p.getNVertices());
    proximo = p.getVertice((i + 1) % p.getNVertices());
    if(anterior.y < atual.y && proximo.y < atual.y){
        return true;
    }
    if(anterior.y > atual.y && proximo.y > atual.y){
        return true;
    }
    return false;
}
//***********************************************************************
int linhaPonto(Ponto p){
    float dif = fabs( p.y - Min.y);
    if(p.y < Min.y || p.y > Max.y){
        return -1;
    }
    float fax = fabs(Max.y - Min.y)/ (Nfaixas - 1);;
    return int(dif/fax);
}
bool dentroConcavOtm(Ponto p, Poligono Concav){
    int intersecs  = 0;
    Ponto inicLinha, fimLinha;
    fimLinha.set(p.x, p.y, 0);
    inicLinha.set(Min.x, p.y, 0);
    int faixa = linhaPonto(p);;
    if(faixa < 0){
        return false;
    }
    for(int i = 0; i < arestas[faixa].size(); i ++){
            int inicio = arestas[faixa][i];
            int fim = (inicio + 1) % Concav.getNVertices();
            if(HaInterseccao(inicLinha, fimLinha, Concav.getVertice(inicio), Concav.getVertice(fim))){
            intersecs ++;
        }
            if(Concav.getVertice(inicio).y == p.y && Concav.getVertice(inicio).x <= p.x){
                if(!maxOUmin(inicio, Mapa)){
                    intersecs ++;
                }
            }
    }
    return intersecs % 2 == 1;
}
//***********************************************************************
bool  dentroConcav(Ponto p, Poligono Concav){
    int intersecs  = 0;
    Ponto inicLinha, fimLinha;
    fimLinha.set(p.x, p.y, 0);
    inicLinha.set(Min.x, p.y, 0);
    for(int i = 0; i < Concav.getNVertices(); i ++){
            if(HaInterseccao(inicLinha, fimLinha, Concav.getVertice(i), Concav.getVertice((i+1)%Concav.getNVertices()))){
            intersecs ++;
        }if(Concav.getVertice(i).y == p.y && Concav.getVertice(i).x <= p.x){
            if(!maxOUmin(i, Mapa)){
                intersecs ++;
            }
        }
    }
    return intersecs % 2 == 1;
}
float RandomNumber(float Min, float Max)
{
    return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}
void desenhaPontos(){
    Ponto p;
    for(int i = 0; i < NPontos; i ++){
        p = pontos[i];
        if(cores[i] == 0){
            glColor3f(0,1,0);
            glBegin(GL_POINTS);
            glVertex3f(p.x, p.y, p.z);
            glEnd();
        }else if(cores[i] == 1){
            glColor3f(1,1,0);
            glBegin(GL_POINTS);
           glVertex3f(p.x, p.y, p.z);
            glEnd();
        }
        else{
            glColor3f(1,0,0);
            glBegin(GL_POINTS);
            glVertex3f(p.x, p.y, p.z);
            glEnd();
        }
    }
}
void geraPontos(){
   float x, y;
   for(int i = 0; i < NPontos; i ++){
        x = RandomNumber(Min.x - 1, Max.x +1);
        y = RandomNumber(Min.y - 1, Max.y + 1);
        Ponto p;
        p.set(x, y, 0);
        pontos[i] = p;
        if(dentroConcavOtm(p, Mapa)){
            cores[i] = 0;
        }else if(dentroConvexo(p, ConvexHull)){
            cores[i] = 1;
        }
        else{
            cores[i] = 2;
        }
   }

}
//***********************************************************************
void LeMapa(const char *nome)
{
    ifstream input;
    input.open(nome, ios::in);
    if (!input)
    {
        cout << "Erro ao abrir " << nome << ". " << endl;
        exit(0);
    }
    cout << "Lendo arquivo " << nome << "...";
    string S;
    int nLinha = 0;
    unsigned int qtdVertices;

    input >> qtdVertices;
    double x,y;

    // Le a primeira linha apenas para facilitar o calculo do limites
    input >> x >> y;

    Min = Ponto(x,y);
    Max = Ponto(x,y);
     Mapa.insereVertice(Ponto(x,y));
    for (int i=1; i< qtdVertices; i++)
    {
        // Le cada elemento da linha
        input >> x >> y;
        // atualiza os limites
        if (x<Min.x) Min.x = x;
        if (y<Min.y) Min.y = y;

        if (x>Max.x) Max.x = x;
        if (y>Max.y) Max.y = y;

        if(!input)
            break;
        nLinha++;
        //cout << "Arquivo: " << x << " " << y << endl;
        Mapa.insereVertice(Ponto(x,y));
    }
    ConvexHull = ConvHull(Mapa);
    criaFaixas();
    geraPontos();
    cout << "leitura concluida." << endl;
    //cout << ConvexHull.getNVertices() << endl;
    //cout << "Linhas lidas: " << nLinha << endl;
    //cout << "Limites:" << endl;
    Min.imprime();
    Max.imprime();
}

// **********************************************************************
//
// **********************************************************************
void init()
{
    // Define a cor do fundo da tela (AZUL)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    LeMapa("EstadoRS.txt");
    Min.x--;Min.y--;
    Max.x++;Max.y++;
    //cout << "Vertices no Vetor: " << Mapa.getNVertices() << endl;

}
// **********************************************************************
//  void init(void)
//  Inicializa os parametros globais de OpenGL
// **********************************************************************
void initOLD(void)
{
	// Define a cor do fundo da tela (AZUL)
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    // Leitura do arquivo
    Mapa.insereVertice(Ponto(0,0));
    Mapa.insereVertice(Ponto(-3,5));
    Mapa.insereVertice(Ponto(5,5));
    Mapa.insereVertice(Ponto(8,-3));
    Mapa.insereVertice(Ponto(-4,-3));

    // Seta os limites da rea de desenho
    Min = Ponto(-10, -10, 0);
    Max = Ponto( 10,  10, 1);
}


double nFrames=0;
double TempoTotal=0;
// **********************************************************************
//
// **********************************************************************
void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    if (AccumDeltaT > 1.0/30) // fixa a atualizao da tela em 30
    {
        AccumDeltaT = 0;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        cout << "Nros de Frames sem desenho: " << nFrames << endl;
        cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
        TempoTotal = 0;
        nFrames = 0;
    }
}
// **********************************************************************
//  void reshape( int w, int h )
//  trata o redimensionamento da janela OpenGL
//
// **********************************************************************
void reshape( int w, int h )
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela area OpenGL dentro da Janela
    glViewport(0, 0, w, h);
    // Define os limites logicos da area OpenGL dentro da Janela
    glOrtho(Min.x,Max.x ,Min.y ,Max.y ,0,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void desenhaFaixas(){
    glBegin(GL_LINES);
    glColor3f(1,1,1);
    for(int i = 0; i < Nfaixas; i ++){
        glVertex2f(Min.x, faixas[i]);
        glVertex2f(Max.x, faixas[i]);
    }
    glEnd();
}
// **********************************************************************
//
// **********************************************************************
void DesenhaEixos()
{
    Ponto Meio;
    Meio.x = (Max.x+Min.x)/2;
    Meio.y = (Max.y+Min.y)/2;
    Meio.z = (Max.z+Min.z)/2;

    glBegin(GL_LINES);
    glColor3f(1, 1, 1);
    //  eixo horizontal
        glVertex2f(Min.x,Meio.y);
        glVertex2f(Max.x,Meio.y);
    //  eixo vertical
        glVertex2f(Meio.x,Min.y);
        glVertex2f(Meio.x,Max.y);
    glEnd();
}
// **********************************************************************
//  void display( void )
//
// **********************************************************************
void display( void )
{

	// Limpa a tela coma cor de fundo
	glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites lógicos da área OpenGL dentro da Janela
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// Coloque aqui as chamadas das rotinas que desenham os objetos
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    glLineWidth(2);
    glColor3f(0,1,0); // R, G, B  [0..1]
    Mapa.desenhaPoligono();

    glColor3f(1,1,0); // R, G, B  [0..1]
    ConvexHull.desenhaPoligono();

    glPointSize(3);
    desenhaPontos();

    glLineWidth(1);
    desenhaFaixas();
    DesenhaEixos();
	glutSwapBuffers();
}
// **********************************************************************
// ContaTempo(double tempo)
//      conta um certo nmero de segundos e informa quanto frames
// se passaram neste perodo.
// **********************************************************************
void ContaTempo(double tempo)
{
    Temporizador T;

    unsigned long cont = 0;
    cout << "Inicio contagem de " << tempo << "segundos ..." << flush;
    while(true)
    {
        tempo -= T.getDeltaT();
        cont++;
        if (tempo <= 0.0)
        {
            cout << "fim! - Passaram-se " << cont << " frames." << endl;
            break;
        }
    }

}
// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
// **********************************************************************

void keyboard ( unsigned char key, int x, int y )
{

	switch ( key )
	{
		case 27:        // Termina o programa qdo
			exit ( 0 );   // a tecla ESC for pressionada
			break;
        case 't':
            ContaTempo(3);
            break;
		default:
			break;
	}
}
// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
//
//
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
		case GLUT_KEY_UP:       // Se pressionar UP
			glutFullScreen ( ); // Vai para Full Screen
			break;
	    case GLUT_KEY_DOWN:     // Se pressionar UP
								// Reposiciona a janela
            glutPositionWindow (50,50);
			glutReshapeWindow ( 700, 500 );
			break;
		default:
			break;
	}
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
// **********************************************************************
int  main ( int argc, char** argv )
{
    cout << "Programa OpenGL" << endl;
    srand(time(NULL));
    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de título da janela.
    glutCreateWindow    ( "Primeiro Programa em OpenGL" );

    // executa algumas inicializações
    init ();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // será chamada automaticamente quando
    // for necessário redesenhar a janela
    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalidao da tela. A funcao "display"
    // será chamada automaticamente sempre que a
    // mquina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // será chamada automaticamente quando
    // o usuário alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // será chamada automaticamente sempre
    // o usuário pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" será chamada
    // automaticamente sempre o usuário
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}
