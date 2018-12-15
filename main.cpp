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
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;

pid_t CHILDid;

/*To fix
 *
 *
 *
 *
 *
 *
 * */

void deleteSpacesFromStart(string &s) {
    int i = 0;
    while (s[i] == ' ')i++;
    s = s.substr(i, s.size());
}

void deleteSpacesFromEnd(string &s) {
    int i = s.size() - 1;
    while (s[i] == ' ')i--;
    s = s.substr(0, i + 1);
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
    deleteSpacesFromEnd(input);
    if (input.empty()) {
        char *home = getenv("HOME");
        struct stat st2;
        if (stat(home, &st2) < 0) {
            fprintf(stderr, "CD error ");
        }
        if (S_ISDIR(st2.st_mode)) {

            int ch = chdir(home);
        } else {
            fprintf(stderr, "%s:Home hadn't been set properly\n", home);
        }
        //cout << '\n';
    } else {
        struct stat st;
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
    printf("%s\n", s);
    fflush(stdout);
}


void execQuant(string &s, int read, int write) {
    deleteSpacesFromEnd(s);
    deleteSpacesFromStart(s);
    if (s.empty()) {
        exit(0);
    } else {
        addSpacesToEnd(s);
        string cmd = getcmd(s);
        vector<char *> arg;
        vector<string> placeholder;
        if (read != 0) {
            close(0);
            dup2(read, 0);
        }
        if (write != 1) {
            close(1);
            dup2(write, 1);
        }
        if (cmd == "pwd") {
            pwd();
            exit(0);
        }
        int ret = 0;
        exclVector(cmd, s, arg, placeholder);
        if (arg[0] != NULL)ret = execvp(cmd.c_str(), &arg[0]);
        if (ret == -1) {

            fprintf(stderr, "Ошибка программы: %s\n", (cmd + s).c_str());
            exit(0);
        }
        exit(0);
    }
}

void readFromFile(string &s, int write) {
    addSpacesToEnd(s);
    string cmd = s.substr(0, s.find('<'));
    s = s.substr(s.find('<') + 1, s.size());
    if (s.find('<') != -1) {
        fprintf(stderr, "Недопустимая команда: %s\n", (cmd + s).c_str());
        exit(0);
    }
    deleteSpacesFromStart(s);
    string name = s.substr(0, s.find(' '));
    close(0);
    open(name.c_str(), O_RDWR | O_CREAT, 0666);
    execQuant(cmd, 0, write);
}

void writeToFile(string &s, int read) {
    addSpacesToEnd(s);
    string cmd = s.substr(0, s.find('>'));
    s = s.substr(s.find('>') + 1, s.size());
    if (s.find('>') != -1 || s.find('<') != -1) {
        fprintf(stderr, "Недопустимая команда: %s\n", (cmd + s).c_str());
        exit(0);
    }
    deleteSpacesFromStart(s);
    string name = s.substr(0, s.find(' '));
    close(1);
    open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    execQuant(cmd, read, 1);
}

void g(string &s) {
    vector<int> pipeArray(1000);
    int i = 0;
    int k = 0;//components counters
    if (s.find('|') != -1) {
        while (s.find('|') != -1) {

            string cmd = s.substr(0, s.find('|'));
            s = s.substr(s.find('|') + 1, s.size());

            if (i == 0 && cmd.find('>') != -1) {
                fprintf(stderr, "Недопустимая команда в компоненте конвеера: %s\n", cmd.c_str());
                exit(0);
            }
            if (i > 0 && (cmd.find('>') != -1 || cmd.find('<') != -1)) {
                fprintf(stderr, "Недопустимая команда в компоненте конвеера: %s\n", cmd.c_str());
                exit(0);
            }
            deleteSpacesFromStart(s);
            addSpacesToEnd(s);
            deleteSpacesFromEnd(cmd);
            if (cmd.empty()) {
                fprintf(stderr, "Нет команды в компоненте конвеера:\n");
            }

            pipe(&pipeArray[i * 2]);
            int id = fork();
            k++;


            if (id == 0) {
                if (k == 1) {

                    if (cmd.find('<') != -1) {
                        readFromFile(cmd, pipeArray[2 * i + 1]);
                    }

                    close(pipeArray[2 * i]);
                    execQuant(cmd, 0, pipeArray[2 * i + 1]);

                } else {
                    close(pipeArray[2 * i - 1]);
                    close(pipeArray[2 * i]);

                    execQuant(cmd, pipeArray[2 * i - 2], pipeArray[2 * i + 1]);

                }
            } else {
                close(pipeArray[2 * i + 1]);
                i++;
                continue;
            }
        }
        close(pipeArray[2 * i - 1]);
        if (s.find('<') != -1) {
            fprintf(stderr, "Недопустимая команда в компоненте конвеера: %s\n", s.c_str());
            exit(0);
        }
        if (s.find('>') != -1) {
            writeToFile(s, pipeArray[2 * i - 2]);
        }

        execQuant(s, pipeArray[2 * i - 2], 1);

    } else {
        if (s.find('>') != -1 && s.find('<') != -1) {
            if (s.find('>') < s.find('<')) {
                writeToFile(s, 0);
            }
            if (s.find('<') < s.find('>')) {
                addSpacesToEnd(s);
                deleteSpacesFromEnd(s);
                string cmd = s.substr(0, s.find('<'));
                s = s.substr(s.find('<') + 1, s.size());
                deleteSpacesFromStart(s);
                if ((s.find('<') != -1) && (s.find('<') < s.find('>'))) {
                    fprintf(stderr, "Недопустимая команда: %s\n", s.c_str());
                    exit(0);
                }
                string name = s.substr(0, s.find('>'));
                deleteSpacesFromStart(name);
                deleteSpacesFromEnd(name);
                close(0);
                close(1);
                s = s.substr(s.find('>') + 1, s.size());
                if (s.find('>') != -1 || s.find('<') != -1) {
                    fprintf(stderr, "Недопустимая команда: %s\n", s.c_str());
                    exit(0);
                }
                deleteSpacesFromStart(s);
                deleteSpacesFromEnd(s);
                open(name.c_str(), O_RDWR | O_CREAT, 0666);
                open(s.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
                execQuant(cmd, 0, 1);

            }
        } else {
            deleteSpacesFromStart(s);

            if (s.find('<') != -1)
                readFromFile(s, 1);

            if (s.find('>') != -1)
                writeToFile(s, 0);

            execQuant(s, 0, 1);
        }

    }
}

void getTime(struct rusage r, struct timeval start) {
    getrusage(RUSAGE_CHILDREN, &r);
    struct timeval UserCpu = r.ru_utime;
    struct timeval SysCpu = r.ru_stime;
    struct timeval end;
    unsigned long long Start =
            (unsigned long long) (start.tv_sec) * 1000000 +
            (unsigned long long) (start.tv_usec);
    gettimeofday(&end, NULL);

    unsigned long long End =
            (unsigned long long) (end.tv_sec) * 1000000 +
            (unsigned long long) (end.tv_usec);

    unsigned long long realMin = (End - Start) / (60000000);
    double realsec = ((double) ((End - Start) % (60000000))) / 1000000;

    double Usertime = UserCpu.tv_sec + (((double) UserCpu.tv_usec) / 1000000);
    double Systime = SysCpu.tv_sec + (((double) SysCpu.tv_usec) / 1000000);
    int Usermin = (int) (Usertime / 60);
    int Sysmin = (int) (Systime / 60);
    double Usersec = Usertime - 60 * Usermin;
    double Syssec = Systime - 60 * Sysmin;
    printf("\n\nreal    %llum%fs\nuser    %dm%fs\nsys     %dm%fs\n", realMin, realsec, Usermin, Usersec, Sysmin,
           Syssec);
}

void pipeline(string &s) {
    if (s.find("time") != -1) {
        struct rusage r;
        s = s.substr(s.find(' ') + 1, s.size());
        struct timeval start;
        gettimeofday(&start, NULL);

        unsigned long long Start =
                (unsigned long long) (start.tv_sec) * 1000000 +
                (unsigned long long) (start.tv_usec);
        pid_t id = fork();
        CHILDid = id;

        if (id == 0) {
            g(s);
        } else {
            int a;
            waitpid(id, &a, 0);
            getTime(r, start);
        }
    } else {
        pid_t id = fork();
        CHILDid = id;
        if (id == 0) {
            g(s);
        } else {
            int a;
            waitpid(id, &a, 0);
        }
    }
}


void sigHandler(int sig) {

    if (sig == SIGINT) {
        kill(CHILDid, 3);
    }
}


int main(int argc, char **argv, char **envp) {

    signal(SIGINT, sigHandler);

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
            string cmd = input.substr(0, input.find(' '));
            if (cmd == "cd") {
                string dir = input.substr(input.find(' '), input.size());
                cd(dir);
            } else pipeline(input);


        } else {
            printf("\n");
            exit(1);
        }
    }

    return 0;
}




