#include <iostream>
#include <vector>
#include <pthread.h>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;
struct symbol
{
    char name;
    double prob;
    double fx;
    double fxbar; 
    double prevfx;
    int *turn;
    pthread_mutex_t *semB;
    pthread_cond_t *waitTurn;
    int threadID;
};
string decBin(double num, int binL) // Decimal fraction to binary
{
    string binary = "";
    int inte = num;
    double fractional = num - inte;
    while (inte)
    {
        int rem = inte % 2;
        binary.push_back(rem + '0');
        inte /= 2;
    }
    reverse(binary.begin(), binary.end());
    binary.push_back('.');
    while (binL--)
    {
        fractional *= 2;
        int fract_bit = fractional;

        if (fract_bit == 1)
        {
            fractional -= fract_bit;
            binary.push_back(1 + '0');
        }
        else
        {
            binary.push_back(0 + '0');
        }
    }
    return binary;
}
void *SFE(void *arg) // Shannon Fano Elias encoding
{
    symbol *s = (symbol *)arg;
    char name = s->name;
    double prob = s->prob;
    double fx = s->fx;
    double prevfx = s->prevfx;
    int turn = *(s->turn);
    int id = s->threadID;
    double fxbar; 
    string fxbin; 
    pthread_mutex_unlock(s->semB);

    fxbar = prob / 2 + prevfx; // pref fx
    int binL = ceil(log2(1 / prob)) + 1; // Find length of binary
    fxbin = decBin(fxbar, binL);
    fxbin = fxbin.erase(0, 1); // Erase leading 0

    //Print
    pthread_mutex_lock(s->semB);
    while((*s->turn) != id)
    {
    pthread_cond_wait(s->waitTurn, s->semB);
    }
    pthread_mutex_unlock(s->semB);

    cout << "Symbol " << name << ", " << "Code: " << fxbin << endl;
    pthread_mutex_lock(s->semB);
    (*s->turn)++;
    pthread_cond_broadcast(s->waitTurn);
    pthread_mutex_unlock(s->semB);


    pthread_exit(NULL);
}

int main()
{
    static pthread_mutex_t semB;
    pthread_mutex_init(&semB, NULL);
    pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER;
    int turn = 1;
    int nthreads;
    
    string s;
    //s = "AAAAAABBBBBCCCCDDDEE";
    string temp;
    vector<char> name;
    vector<double>prob;
    vector<double>prevfx;
    vector<double>fx;
    struct symbol data;
    data.semB = &semB;
    data.waitTurn = &waitTurn;
    data.turn = &turn;
    int count = 0; //number of threads needed
    cin >> s; //Get input from user 
    map<char, double >m;
    map<char, double >::iterator itr;

    for (int i = 0; i < s.length(); i++) 
    {
        m[s[i]]++; //Calcuate frequency 
    }
    for (itr = m.begin(); itr != m.end(); itr++) 
    {
        double p = itr->second/s.length(); //Calculate prob
        name.push_back(itr->first);
        prob.push_back(p);
        count++;     
    }
    //Calculate Fx
    for (int i = 0; i < count; i++)
    {
        if (i == 0)
        {
            fx.push_back(0 + prob[i]);
            prevfx.push_back(0);
        }
        else
        {
            fx.push_back(fx[i - 1] + prob[i]);
            prevfx.push_back(fx[i - 1]);
        }
    }

    //Threads creation and join
    pthread_t thread_id[count];
    cout << "SHANNON-FANO-ELIAS Codes:\n";
    
    for (int i = 0; i < count; i++)
    {        
        pthread_mutex_lock(&semB);
        data.name = name[i];
        data.prob = prob[i];
        data.fx  = fx[i];
        data.prevfx = prevfx[i];
        data.threadID = i + 1;
        pthread_create(&thread_id[i], NULL, SFE, &data);
    }
    for (int j = 0; j < count; j++)
    {
        pthread_join(thread_id[j], NULL);
    }

    return 0;
}
