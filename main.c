#include<stdio.h>
#include<signal.h>
#include<error.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/times.h>
#include<sys/wait.h>

typedef struct {
    double real;
    double sys;
    double user;
} Times;

long clocks_per_sec;

Times getTime(clock_t start) {
    struct tms tms;
    Times clocks;
    clock_t end;
    end = times(&tms);
    clocks.real = (double)(end - start) / clocks_per_sec;
    clocks.sys = (double)tms.tms_cstime / clocks_per_sec;
    clocks.user = (double)tms.tms_cutime / clocks_per_sec;
    return clocks;
}

int run(char *argv[]) {
    pid_t pid;
    int status;
    pid = fork();
    if(pid < 0) {
        error(2, errno, "Failed to fork.");
    } else if(pid == 0) {
        execvp(argv[0], argv);
        error(3, errno, "Failed to run '%s'.", argv[0]);
    }
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    wait(&status);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    return status;
}

void printTime(const char *title, double tms) {
    long minutes;
    double seconds;
    minutes = tms / 60.0;
    seconds = tms - 60.0 * minutes;
    printf("%s %ldm%.3fs\n", title, minutes, seconds);
}

int main(int argc, char *argv[]) {
    int status;
    Times tms;
    struct tms startTms;
    clock_t start;
    if(argc < 2) error(1, 0, "Specify a target command.");
    clocks_per_sec = sysconf(_SC_CLK_TCK);
    start = times(&startTms);
    status = run(&argv[1]);
    tms = getTime(start);
    puts("");
    printTime("real", tms.real);
    printTime("usr", tms.user);
    printTime("sys", tms.sys);
    return status;
}