#include <iostream>
#include <unistd.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>

using namespace std;

string getCwd() {
    char *add = (char *) malloc(sizeof(char) * 10000);
    string s(getcwd(add, 10000));//расширить про очищение памяти в такой ситуации
    return s;
}

vector<char *> exclVector(string &cmd, string &input) {//формирует вектор аргументов(так как они пришли)
    vector<string> placeholder;
    vector<char *> arg;
    placeholder.push_back(cmd);

    int k = 0;
    while (!input.empty()) {
        string s(input.substr(0, input.find(' ')));
        placeholder.push_back(s);
        if (input.find(' ') + 1 != 0)
            input = input.substr(input.find(' ') + 1, input.size());
        else input = "";
        k++;
    }
    for (int i = 0; i <= k; ++i) {
        arg.push_back((char *) placeholder[i].c_str());
    }
    arg.push_back(NULL);
    return arg;
}

void time(string &cmd, string &input) {

    vector<char *> arg = exclVector(cmd, input);
    arg.push_back(NULL);

    string a = "time";
    string path = "/usr/bin/" + a;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(path.c_str(), &arg[0]);
    } else {
        int info;
        waitpid(pid, &info, 0);//родитель ждёт исполненя ребёнка
    }
}

void cd(string &input) {
    if (input.empty()) {
        char *home = getenv("HOME");
        int ch = chdir(home);

        cout << '\n';

    } else {
        int ch = chdir(input.c_str());
        cout << '\n';
    }
}

//перенаправления вывода одного процесса на ввод другого
void outRederection(string &command1, vector<char *> &args1, string &filename) {
    pid_t pid = fork();
    if (pid == 0) {
        close(1);//closing out descriptor

        const char *name = filename.c_str();
        open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
        string path;
        if(command1!="time"){path = "/bin/" + command1;}
        else{
           path = "/usr/bin/time";
        }
        execvp(path.c_str(), &args1[0]);
    } else {
        int info;
        waitpid(pid, &info, 0);
    }
}

string getcmd(string &input) {//returns first word and truncates it
    string cmd = input.substr(0, input.find(' '));
    if (input.find(' ') + 1 != 0)
        input = input.substr(input.find(' ') + 1, input.size());
    else input = "";
    return cmd;
}

int main(int argc, char **argv, char **envp) {
    /*for (int i = 0; i <argc; ++i) {

        cout<<argv[i];
    }
    putchar('\n');
    for (int i = 0; envp[i]!=NULL; ++i) {
       printf("%s\n",envp[i]);

    }*/
    string input = "2";

    uid_t a = getuid();
    char p;
    if (a == 0) {
        p = '!';
    } else {
        p = '>';
    }
    int x = 1;
    while (x) {
        string dir = getCwd();
        cout << dir << p;
        getline(cin, input);

        if (input.find('>') != -1) {
            string s1 = input.substr(0, input.find('>'));
            string s2 = input.substr(input.find('>') + 1, input.size());
            string command1 = getcmd(s1);
            vector <char *>arg = exclVector(command1,s1);
            outRederection(command1,arg,s2);

        } else {
            string cmd = getcmd(input);


            if (cmd == "cd") {
                cd(input);
            } else if (cmd == "pwd") {
                cout << dir;
            } else if (cmd == "time") {//работает не всегда корректно
                time(cmd, input);
            } else {//внешняя команда
                vector<char *> arg = exclVector(cmd, input);
                string path = "/bin/" + cmd;

                pid_t pid = fork();
                if (pid == 0) {
                    execvp(path.c_str(), &arg[0]);
                } else {
                    int info;
                    waitpid(pid, &info, 0);//родитель ждёт исполненя ребёнка
                }
            }

        }
    }


    return 0;
}
