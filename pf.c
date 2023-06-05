/* ********************* *
 * TP1 INF3173 H2021
 * Code permanent: HAUJ21049307
 * Nom: Hauchecorne
 * Prénom: Jules
 * ********************* */

#include <a.out.h>
#include <asm/unistd.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

bool opt_u = false;
bool opt_c = false;
bool opt_a = false;
bool opt_n = false;
bool opt_s = false;
short nbr_n = 1;
short nbrOpt;
double tempsReel = 0;
double tempsUtilisateur = 0;
long long nbrCycleCpu = 0;
double tempsReelTotal = 0;
double tempsUtililTotal = 0;
long long nbrCycleCpuTotal = 0;

// Calcul la moyenne des valeurs de temps ou de cycle cpu
// selon l'option -n et l'affiche.
void traitementMoyenne() {
    tempsReelTotal = tempsReelTotal / nbr_n;
    tempsUtililTotal = tempsUtililTotal / nbr_n;
    nbrCycleCpuTotal = nbrCycleCpuTotal / nbr_n;
    if (opt_a) {
        printf("%0.2f %0.2f %lld\n", tempsReelTotal, tempsUtililTotal,
                nbrCycleCpuTotal);
    } else if (opt_u) {
        printf("%0.2f\n", tempsUtililTotal);
    } else if (opt_c) {
        printf("%lld\n", nbrCycleCpuTotal);
    } else {
        printf("%0.2f\n", tempsReelTotal);
    }
}

// Affichage du temps réel, utilisateur ou cycles cpu selon les options passées au programme.
int affichage() {
    if (opt_a) {
        printf("%0.2f %0.2f %lld\n", tempsReel, tempsUtilisateur, nbrCycleCpu);
    } else if (opt_u) {
        printf("%0.2f\n", tempsUtilisateur);
    } else if (opt_c) {
        printf("%lld\n", nbrCycleCpu);
    } else {
        printf("%0.2f\n", tempsReel);
    }
    return 0;
}

// Méthode reprise du man page de perf_event_open qui sert à définir
// l'enclanchement de perf et retourne une valeur négative si l'appel système
// échoue.
static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu,
        int group_fd,
        unsigned long flags) {                                           // From man of perf_event_open
    int ret;                                                             // From man of perf_event_open
    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd,    // From man of perf_event_open
            flags);                                                      // From man of perf_event_open
    return ret;                                                          // From man of perf_event_open
}

// Execute un la commande passé en argument dans un fork, le temps uitlisateur
// ou réel est calculé. Si l'execution échoue un message d'erreur est affiché et
// le programme se termine avec un code de retour de 127.
int executeProcessus(char *process[]) {
    struct timespec start, end;
    struct rusage usage;
    int status;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        if (opt_s) {
            if (execlp("/bin/sh", "/bin/sh", "-c", *process, NULL) == -1) {
                exit(127);
            }
        }
        if (execvp(*process, process) == -1) {
            exit(127);
        }
    }
    wait4(0, &status, 0, &usage);
    waitpid(0, &status, 0);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (status != 256 && status != 0) {
        fprintf(stderr, "Erreur d'execution de la commande passée au programme!\n");
        exit(127);
    } else if (status == 256) {
        printf("%0.2f", tempsUtilisateur);
        exit(-1);
    }
    tempsUtilisateur = (double)(usage.ru_utime.tv_sec + (usage.ru_utime.tv_usec / 1000000.0));
    tempsReel = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
    tempsReelTotal = tempsReelTotal + tempsReel;
    tempsUtililTotal = tempsUtililTotal + tempsUtilisateur;
    return 0;
}

// Identifie les arguments passés au programme et fait le compte du nombre d'options total.
void identifierAguments(char *args[]) {
    short i = 1;
    while (strchr(args[i], '-') != NULL) {
        if (strcmp(args[i], "-u") == 0) {
            opt_u = true;
        }
        if (strcmp(args[i], "-c") == 0) {
            opt_c = true;
        }
        if (strcmp(args[i], "-a") == 0) {
            opt_a = true;
        }
        if (strcmp(args[i], "-n") == 0) {
            opt_n = true;
            nbr_n = atoi(args[i + 1]);
            i++;
        }
        if (strcmp(args[i], "-s") == 0) {
            opt_s = true;
        }
        i++;
    }
    nbrOpt = i;
}

int main(int argc, char *argv[]) {
    struct perf_event_attr pe;                      // From man of perf_event_open
    long long count;                                // From man of perf_event_open
    int fd;                                         // From man of perf_event_open
    memset(&pe, 0, sizeof(struct perf_event_attr)); // From man of perf_event_open
    pe.type = PERF_TYPE_HARDWARE;                   // From man of perf_event_open
    pe.size = sizeof(struct perf_event_attr);       // From man of perf_event_open
    pe.config = PERF_COUNT_HW_CPU_CYCLES;           // From man of perf_event_open
    pe.exclude_kernel = 1;   // to work with perf_event_paranoid at 2 and not only at 1.
    pe.inherit = 1;                          // to add child process to counts
    fd = perf_event_open(&pe, 0, -1, -1, 0);        // From man of perf_event_open
    if (fd == -1) {
        fprintf(stderr, "\nError with perf_event_open\n");
        exit(EXIT_FAILURE);
    }
    identifierAguments(argv);
    for (short x = 0; x < nbr_n; x++) {
        ioctl(fd, PERF_EVENT_IOC_RESET, 0);  // From man of perf_event_open
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0); // From man of perf_event_open
        executeProcessus(argv + nbrOpt);
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0); // From man of perf_event_open
        read(fd, &count, sizeof(long long));  // From man of perf_event_open
        nbrCycleCpu = count;
        nbrCycleCpuTotal = nbrCycleCpuTotal + nbrCycleCpu;
        affichage();
    }
    close(fd);
    if (nbr_n > 1) {
        traitementMoyenne();
    }
    return 0;
}
