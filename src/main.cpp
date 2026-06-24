# include <iostream>
# include <unistd.h>

using namespace std;

int main(int argc, char *argv[]){

    cout << "===== SYSLENS =====" << endl;
    cout << "PID: " << getpid() << endl;
    cout << "PPID: " << getppid() << endl;

    return 0;
}