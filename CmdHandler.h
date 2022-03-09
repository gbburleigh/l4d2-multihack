#pragma once
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <vector>

using namespace std;



#ifndef CmdHandler_H
#define CmdHandler_H
#define A2DD_H
class Command {
    string cmd;
    bool createThread;
    vector<string> args;
public:
    Command(string c, bool create, vector<string> argv);
    void Execute();
};

#endif

 void SetStdinEcho(bool enable);