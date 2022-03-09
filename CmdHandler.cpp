#include "CmdHandler.h"
#include <functional>
#include <unordered_map> 
#include <memory>

using namespace std;

Command::Command(string c, bool create, vector<string> argv) {
    cmd = c;
    createThread = create;
    args = argv;
}

//const char TOGGLE_CMD[] = "toggle";
//void Command::Execute() {
//    switch (hash(cmd)){
//    case hash(TOGGLE_CMD):
//            break;
//    }
//}

void SetStdinEcho(bool enable = true)
{
#ifdef WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if (!enable)
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode);

#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if (!enable)
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

//int main(int argc, const char argv[]) {
//    SetStdinEcho(false);
//    /*while (true) {
//        char c;
//        while (c != '\n') {
//            std::cin >> c;
//            std::cout << c;
//        }
//    }*/
//}