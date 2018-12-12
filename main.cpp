#include <iostream>
#include <unistd.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>
#include <cstring>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

/*To fix
 * Обработать /(apparently works, need to test)
 * >|>
 * Добавить внутренние команды на конвеер
 *Время
 *Сигналы
 * Навести красоту
 * */
void deleteSpacesFromStart(string &s) {
    int i = 0;
    while (s[i] == ' ')i++;
    s = s.substr(i, s.size());
}

void addSpacesToEnd(string &s) {//Returns string with one space at the end.
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

bool empty(string &s) {
    for (int i = 0; i < s.size(); ++i) {
        if (s[i] != ' ')return false;
    }
    return true;
}

int exclVector(string &cmd, string &input, vector<char *> &arg,
               vector<string> &placeholder) {//формирует вектор аргументов(так как они пришли)
    placeholder.push_back(cmd);
    bool search = false;
    int y = 0;
    string copy = input;
    while (!input.empty()) {
        unsigned long l = input.find(' ');
        string s(input.substr(0, l));
        if ((s.find('*') == -1) && (s.find('?') == -1) && (s.find('/') == -1)) {

            placeholder.push_back(s);


        } else {
            copy = s;
            if (s.find('/') != -1) {
                if (s.find('*') == -1 && s.find('?') == -1) {
                    placeholder.push_back(s);
                } else {
                    search = true;
                    deleteSpacesFromStart(s);
                    if (s[0] != '/' && s[0] != '.') {
                        fprintf(stderr, "%s: Нет такого файла или каталога\n", s.c_str());
                    } else {

                        string path = s.substr(0, s.find('/') + 1);
                        s = s.substr(s.find('/') + 1, s.size());


                        vector<string> DirToCheck;
                        vector<string> DirToCheck2;
                        DirToCheck.push_back(path);
                        int k = 0;
                        while (s.find('/') != -1) {

                            string pattern = s.substr(0, s.find('/'));
                            while (!DirToCheck.empty()) {
                                path = DirToCheck.back();
                                DirToCheck.pop_back();
                                struct stat st;
                                if (stat(path.c_str(), &st) < 0) {
                                    fprintf(stderr, "%s: Нет такого файла или каталога\n", path.c_str());
                                    exit(1);
                                }
                                if (S_ISDIR(st.st_mode)) {

                                    DIR *d = opendir(path.c_str());
                                    if (d == NULL) {
                                        if (errno == EACCES) {
                                            //  fprintf(stderr, "%s: В доступе отказано\n", path.c_str());
                                        }
                                        if (errno == ENOTDIR) {
                                            fprintf(stderr, "%s: Не директория\n", path.c_str());
                                        }

                                        continue;
                                    }
                                    for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {

                                        if (string(de->d_name) == ".") continue;
                                        if (string(de->d_name) == "..") continue;
                                        if (!fnmatch(pattern.c_str(), de->d_name, 0) &&
                                            (de->d_type == DT_DIR)) {
                                            string g;
                                            if (k == 0) {
                                                g = path + de->d_name;
                                            } else { g = path + "/" + de->d_name; }

                                            DirToCheck2.push_back(g);
                                        }
                                    }

                                }

                            }
                            k++;
                            while (!DirToCheck2.empty()) {
                                DirToCheck.push_back(DirToCheck2.back());
                                DirToCheck2.pop_back();
                            }

                            s = s.substr(s.find('/') + 1, s.size());
                        }
                        //Обрабатываю файлы в последних директориях
                        while (!DirToCheck.empty()) {
                            path = DirToCheck.back();
                            DirToCheck.pop_back();

                            string pattern = s;//Возможно есть пробелы в конце

                            struct stat st;
                            if (stat(path.c_str(), &st) < 0) {
                                fprintf(stderr, "%s: Нет такого файла или каталога\n", path.c_str());
                                continue;
                            }
                            if (S_ISDIR(st.st_mode)) {

                                DIR *d = opendir(path.c_str());
                                if (d == NULL) {
                                    if (errno == EACCES) {
                                        //fprintf(stderr, "%s: В доступе отказано\n", path.c_str());
                                    }
                                    continue;
                                }
                                for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {

                                    if (string(de->d_name) == ".") continue;
                                    if (string(de->d_name) == "..") continue;
                                    if (!fnmatch(pattern.c_str(), de->d_name, 0)) {
                                        string g = path + "/" + de->d_name;
                                        placeholder.push_back(g);
                                        y = true;
                                    }
                                }
                            }
                            //нужно закидывать и файлы и папки


                        }


                    }
                }
            } else {
                DIR *d = opendir(".");
                deleteSpacesFromStart(s);

                for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {

                    if (string(de->d_name) == ".") continue;
                    if (string(de->d_name) == "..") continue;
                    if (!fnmatch(s.c_str(), de->d_name, 0)) {
                        string g = de->d_name;
                        placeholder.push_back(g);
                        y = true;
                    }
                }

            }
        }


        unsigned long i = l;
        while ((i < input.size()) && input[i] == ' ')i++;


        input = input.substr(i, input.size());

    }

    if (empty(copy)) {
        for (int i = 0; i < placeholder.size(); ++i) {
            arg.push_back((char *) placeholder[i].c_str());
        }

    } else {
        if (search && !y) {
            fprintf(stderr, "%s: Нет такого файла или каталога\n", copy.c_str());
        } else {
            for (int i = 0; i < placeholder.size(); ++i) {
                arg.push_back((char *) placeholder[i].c_str());
            }
        }
    }

    arg.push_back(NULL);
    return 1;
}

void cd(string &input) {

    deleteSpacesFromStart(input);
    if (input.empty()) {
        char *home = getenv("HOME");
        struct stat st2;
        if (stat(home, &st2) < 0) {
            fprintf(stderr, "CD error");
        }
        if (S_ISDIR(st2.st_mode)) {

            int ch = chdir(home);
        } else {
            fprintf(stderr, "%s:Home hadn't been set properly\n", home);
        }
        //cout << '\n';
    } else {
        struct stat st;
        input = input.substr(0, input.size() - 1);
        if (stat(input.c_str(), &st) < 0) {
            fprintf(stderr, "CD error");
        }

        if (S_ISDIR(st.st_mode)) {
            int ch = chdir(input.c_str());
            if (ch == -1)fprintf(stderr, "Error Code%d\n", errno);
            //cout << '\n';
        } else {
            fprintf(stderr, "%s:Not a directory\n", input.c_str());
        }
    }
}

void pwd() {
    char add[1000];
    char *s;
    s = getcwd(add, 1000);
    printf("%s%c", s);
    fflush(stdout);
}


//Для записи: open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
//Для чтения: open(name, O_RDWR | O_CREAT, 0666);
//перенаправления вывода одного процесса на ввод другого
void rederection() {

}

void execQuant(string &s,int read, int write){
    addSpacesToEnd(s);
    string cmd = getcmd(s);
    vector<char *> arg;
    vector<string> placeholder;
    if (read!=0){
        close(0);
        dup2(read,0);
    }
    if (write!=0) {
        close(1);
        dup2(write, 1);
    }

    if (cmd == "time") {

    }
    if (cmd == "pwd") {
        pwd();
        exit(0);
    }
    if (cmd == "cd") {
        cd(s);
        exit(0);
    }
    int ret = 0;
    exclVector(cmd, s, arg, placeholder);
    if (arg[0] != NULL)ret = execvp(cmd.c_str(), &arg[0]);
    if (ret == -1)fprintf(stderr, "Exec doesn't work. id: %d", getpid());
}

void g(string &s) {
    int ret = 0;
    vector<int> processList;
    vector<int> pipeArray(1000);
    int i = 0;
    if (s.find('|') != -1) {
        while (s.find('|') != -1) {

            string cmd = s.substr(0, s.find('|'));
            s = s.substr(s.find('|') + 1, s.size());
            deleteSpacesFromStart(s);
            pipe(&pipeArray[i * 2]);
            int id = fork();
            processList.push_back(id);

            if (id == 0) {
                if (processList.size() == 1) {
                    close(pipeArray[2 * i]);
                    close(1);
                    dup2(pipeArray[2 * i + 1], 1);
                    execQuant(cmd,0,pipeArray[2*i+1]);

                    string cm = getcmd(cmd);
                    vector<string> placeholder;
                    vector<char *> arg;
                    exclVector(cm, cmd, arg, placeholder);
                    string path = cm;
                    if (arg[0] != NULL) { ret = execvp(path.c_str(), &arg[0]); }
                    else {
                        exit(0);
                    }
                    if (ret == -1) {
                        fprintf(stderr, "Exec doesn't work. id: %d", getpid());
                    }
                    ret = 0;
                } else {

                    close(0);
                    close(pipeArray[2 * i - 1]);
                    close(pipeArray[2 * i]);
                    dup2(pipeArray[2 * i - 2], 0);
                    close(1);

                    dup2(pipeArray[2 * i + 1], 1);
                    addSpacesToEnd(cmd);
                    string cm = getcmd(cmd);
                    vector<char *> arg;
                    vector<string> placeholder;
                    exclVector(cm, cmd, arg, placeholder);

                    string path = cm;
                    if (arg[0] != NULL)ret = execvp(path.c_str(), &arg[0]);
                    if (ret == -1) {
                        fprintf(stderr, "Exec doesn't work. id: %d errno: %d\n", getpid(), errno);
                    }
                    ret = 0;
                }
            } else {

                close(pipeArray[2 * i + 1]);
                i++;
                continue;
            }
        }

        ret = 0;
        close(0);
        close(pipeArray[2 * i - 1]);
        dup2(pipeArray[2 * i - 2], 0);
        addSpacesToEnd(s);
        string cmd = getcmd(s);
        vector<char *> arg;
        vector<string> placeholder;
        exclVector(cmd, s, arg, placeholder);
        if (arg[0] != NULL)ret = execvp(cmd.c_str(), &arg[0]);
        if (ret == -1)fprintf(stderr, "Exec doesn't work. id: %d", getpid());
    } else {
        if (s.find('>') == -1 && s.find('<') == -1) {


        } else {
            while (!s.empty()) {
                if (s.find('>') != -1 && s.find('<') != -1) {
                    if (s.find('>') < s.find('<')) {

                    }
                    if (s.find('<') < s.find('>')) {

                    }
                }
            }
        }
    }
}

void pipeline(string &s) {
    pid_t id = fork();
    if (id == 0) {
        g(s);

    } else {
        int a;
        waitpid(id, &a, 0);
    }
}

int main(int argc, char **argv, char **envp) {

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
        s = getcwd(add, 1000);
        printf("%s%c", s, p);
        fflush(stdout);

        string input;
        if (!getline(cin, input).fail()) {
            deleteSpacesFromStart(input);
            addSpacesToEnd(input);
            pipeline(input);


        } else {
            printf("\n");
            exit(1);
        }
    }
}