#include <iostream>
#include <unistd.h>
#include <vector>
#include <wait.h>

using namespace std;

string getCwd() {
    char *add = (char *) malloc(sizeof(char) * 10000);
    string s(getcwd(add, 10000));//расширить про очищение памяти в такой ситуации
    return s;
}

vector <char *> exclVector(string&cmd, string&input){//формирует вектор аргументов(так как они пришли)
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

        string cmd = input.substr(0, input.find(' '));
        if (input.find(' ') + 1 != 0)
            input = input.substr(input.find(' ') + 1, input.size());
        else input = "";

        if (cmd == "cd") {
            cd(input);
        } else
        if (cmd == "pwd") {
            cout << dir;
        } else
        if (cmd == "time") {//работает не всегда корректно
            time(cmd, input);
        } else {//внешняя команда
            vector<char *> arg= exclVector(cmd,input);
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


    return 0;
}
