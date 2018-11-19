#include <iostream>
#include <unistd.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>
#include <cstring>

using namespace std;



void deleteSpacesFromStart(string& s){
    int i=0;
    while (s[i]==' ')i++;
    s = s.substr(i,s.size());
}
void addSpacesToEnd(string &s) {
    s = s + " ";
    unsigned long i = s.size();
    while (i > 0 && (s[i - 1] == ' ')) {

        i--;
    }
    s = s.substr(0, i + 1);
}

string getcmd(string &input) {//returns first word and truncates it
    unsigned long s = input.find(' ');
    string cmd = input.substr(0, s);
    unsigned long i = s;
    while ((i < input.size()) && input[i] == ' ')i++;

    input = input.substr(i, input.size());


    return cmd;
}

void exclVector(string &cmd, string &input,vector<char *>&arg, vector<string>&placeholder) {//формирует вектор аргументов(так как они пришли)
    placeholder.push_back(cmd);

    int k = 0;
    while (!input.empty()) {
        unsigned long l = input.find(' ');
        string s(input.substr(0, l));
        placeholder.push_back(s);
        unsigned long i = l;
        while ((i < input.size()) && input[i])i++;


        k++;

        input = input.substr(i, input.size());
    }
    for (int i = 0; i <= k; ++i) {
        arg.push_back((char *) placeholder[i].c_str());
    }
    arg.push_back(NULL);
}

void time(string &cmd, string &input) {
    vector<string>placeholder;
    vector<char *> arg;
            exclVector(cmd, input,arg,placeholder);
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
        if (command1 != "time") { path = "/bin/" + command1; }
        else {
            path = "/usr/bin/time";
        }
        execvp(path.c_str(), &args1[0]);
    } else {
        int info;
        waitpid(pid, &info, 0);
    }
}
//notstable
/*
void f(string &s) {
    fprintf(stderr, "%s\n", &s[0]);
    vector<int> processList;
    vector<int> pipeArray(1000);
    int i = 0;

    while (s.find('|') != -1) {
        int ret = 0;
        pipe(&pipeArray[i]);

        string cmd = s.substr(0, s.find('|'));
        s = s.substr(s.find('|') + 1, s.size());

        pid_t id = fork();
        //fprintf(stderr, "Hi, My child is: %d\n", id);
        processList.push_back(id);
        fprintf(stderr, "My id:%d %sMy child id: %d\n", getpid(), &s[0], id);
        // printf("Hi,I'm %d\n",getpid());
        if (id == 0) {
            if (processList.size() == 1) {
                close(pipeArray[i]);
                close(1);
                dup2(pipeArray[i + 1], 1);//теперь дескриптор вывода показывет на вход pip'a
                addSpacesToEnd(cmd);
                string cm = getcmd(cmd);
                string path = "/bin/" + cm;


                vector<char *> arg = exclVector(cm, cmd);
                fprintf(stderr, "First is launched: %d\n", getpid());
                execvp(path.c_str(), &arg[0]);
            } else {
                //fprintf(stderr, "Hi, I'm second: %d\n", getpid());
                int a;
                if (processList.size() > 1) waitpid(processList[processList.size() - 2], &a, 0);

                close(pipeArray[i]);
                close(0);
                dup2(pipeArray[i - 2], 0);
                close(1);
                dup2(pipeArray[i + 1], 1);//теперь дескриптор вывода показывет на вход pip'a
                addSpacesToEnd(cmd);
                string cm = getcmd(cmd);
                string path = "/bin/" + cm;


                vector<char *> arg = exclVector(cm, cmd);
                ret = 0;
                fprintf(stderr, "Another is launched: %d\n", getpid());
                ret = execvp(path.c_str(), &arg[0]);
                if (ret == -1)fprintf(stderr, "LOL %d Arguments:%s\n", getpid(), cm.c_str());
                exit(1);
            }

        } else {

            //printf("I'm parent\n");
            //printf("%s\n", &s[0]);
            int a;

            if (s.find('|') == -1) {
                //fprintf(stderr, "Hi,I'm Out of the loop %d, i=%d\n", getpid(),i);

                if (processList.size() > 0)waitpid(processList[processList.size() - 1], &a, 0);
                close(pipeArray[i + 1]);
                close(0);
                addSpacesToEnd(cmd);
                dup2(pipeArray[i], 0);
                string cm = getcmd(s);
                string path = "/bin/" + cm;
                //   fprintf(stderr, "%s\n",&path[0]);
                vector<char *> arg = exclVector(cm, s);
                //    fprintf(stderr, "%s\n",&arg[0][0]);
                //fprintf(stderr, "%s\n", arg[0]);
                fprintf(stderr,"Final step\n");
                execvp(path.c_str(), &arg[0]);
            }
            //waitpid(id,&a,0);//тут будет чудестный дедлок
            //    fprintf(stderr, "Hi There, Я дождался сына: %d\n", id);
            close(pipeArray[i + 1]);
            i = i + 2;
            continue;
        }

    }
    // printf("Hi, My child is: %d\n",id);



}
*/

void g(string &s) {
    int ret = 0;
    vector<int> processList;
    vector<int> pipeArray(1000);
    int i = 0;
    while (s.find('|') != -1) {

        string cmd = s.substr(0, s.find('|'));
        s = s.substr(s.find('|') + 1, s.size());
        deleteSpacesFromStart(s);
        pipe(&pipeArray[i*2]);
        int id = fork();
        //printf("I'm: %d\n",getpid());
        processList.push_back(id);

        if (id == 0) {
            if (processList.size() == 1) {
                close(pipeArray[2*i]);
                close(1);
                dup2(pipeArray[2*i+1], 1);
                addSpacesToEnd(cmd);
                string cm = getcmd(cmd);
                vector<string>placeholder;
                vector<char *> arg;
                exclVector(cm, cmd,arg,placeholder);
                string path = "/bin/" + cm;
                ret = execvp(path.c_str(), &arg[0]);
                if (ret == -1)fprintf(stderr,"Exec doesn't work. id: %d",getpid());
                ret = 0;
            } else {
                if (processList.size() > 2) {
                   // printf("AAA %d\n",getpid());
                    int a;
                    waitpid(processList[processList.size() - 3], &a, 0);
                }
                close(0);
                close(pipeArray[2*i-1]);
                close(pipeArray[2*i]);
                dup2(pipeArray[2*i-2], 0);
                close(1);

                dup2(pipeArray[2*i+1], 1);
                addSpacesToEnd(cmd);
                string cm = getcmd(cmd);
                vector<char *> arg;
                vector<string>placeholder;
                exclVector(cm, cmd,arg,placeholder);

                string path = "/bin/" + cm;
                ret = execvp(path.c_str(), &arg[0]);
                if (ret == -1) {
                    fprintf(stderr, "Exec doesn't work. id: %d errno: %d\n", getpid(),errno);
                }
                ret = 0;
            }
        } else {
            close(pipeArray[2*i+1]);
            i++;
            continue;
        }
    }
    if (processList.size() >= 2) {

        int a;
        waitpid(processList[processList.size() - 2], &a, 0);
    }
    ret = 0;
    close(0);
    close(pipeArray[2*i-1]);
    dup2(pipeArray[2*i-2],0);
    addSpacesToEnd(s);
    fprintf(stderr,"Finish\n");
    string cm = getcmd(s);
    vector<char *> arg;
    vector<string>placeholder;
    exclVector(cm, s,arg,placeholder);
    string path = "/bin/" + cm;
    ret = execvp(path.c_str(), &arg[0]);
    if (ret == -1)fprintf(stderr,"Exec doesn't work. id: %d",getpid());
}

void pipeline(string &s) {
    pid_t id = fork();
    if (id == 0) {
        //fprintf(stderr, "Going in %d\n", getpid());
        g(s);

    } else {
        int a;
        waitpid(id, &a, 0);
        fprintf(stderr, "I met my child: %d\n", id);
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
    uid_t a = getuid();
    char p;
    if (a == 0) {
        p = '!';
    } else {
        p = '>';
    }
    int x = 1;
    while (x) {
        char add[1000];
        char *s;
        s = getcwd(add,1000);
        printf("%s%c",s,p);

        string input;
        getline(cin, input);
        deleteSpacesFromStart(input);
        addSpacesToEnd(input);

        if (input.find('>') != -1) {
            string s1 = input.substr(0, input.find('>'));
            string s2 = input.substr(input.find('>') + 1, input.size());
            string command1 = getcmd(s1);
            vector<char *> arg;
            vector<string>placeholder;
            exclVector(command1, s1,arg,placeholder);

            outRederection(command1, arg, s2);

        } else {
            if (input.find('|') != -1) {

                pipeline(input);

            } else {
                string cmd = getcmd(input);
                if (cmd == "cd") {
                    cd(input);
                } else if (cmd == "pwd") {
                    printf("%s",s);
                } else if (cmd == "time") {//работает не всегда корректно
                    time(cmd, input);
                } else {//внешняя команда
                    vector<char *> arg;
                    vector<string>placeholder;
                    exclVector(cmd, input,arg,placeholder);
                    string path = "/bin/" + cmd;
                    pid_t pid = fork();
                    if (pid == 0) {
                        execv(path.c_str(), &arg[0]);
                    } else {
                        int info;
                        waitpid(pid, &info, 0);//родитель ждёт исполненя ребёнка
                    }
                }
            }
        }

    }
    return 0;
}