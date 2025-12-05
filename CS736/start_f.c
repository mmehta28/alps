#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/wait.h>

#define SYS_NEWEXECVE 449

struct WorkloadEntry {
    int id;
    int para;
    char func_name[64];
    int time_interval;
    int func_class;
};

int main() {
    FILE *file = fopen("workload.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    struct WorkloadEntry entry;
    int idx = 0;
    while (fscanf(file, "%d %d %s %d %d", &entry.id, &entry.para, entry.func_name, &entry.time_interval, &entry.func_class) == 5) {
        printf("Executing workload entry %d (func: %s, para: %d, func_class: %d)\n", entry.id, entry.func_name, entry.para, entry.func_class);

        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("Error forking child process");
            fclose(file);
            return EXIT_FAILURE;
        }

        if (child_pid == 0) { 
            char filename[256] = "bazel-bin/simple_exp"; 
            //strcat(filename, entry.func_name); 

            char para_str[32];
            snprintf(para_str, sizeof(para_str), "%d", entry.para);
            char id_str[32];
            snprintf(id_str, sizeof(id_str), "%d", entry.id);
            char func_str[32];
            snprintf(func_str, sizeof(func_str), "%d", entry.func_class);
            char *const cmdline[] = {filename, id_str, para_str, func_str, NULL}; // 命令行参数
            char *const envp[] = {"VAR=value", NULL}; 
            int pred = entry.func_class;

            long ret = execve(filename, cmdline, envp);
            if (ret == -1) {
                perror("Error calling sys_newexecve syscall");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        } else { 
            usleep(entry.time_interval * 1000);
        }
   idx += 1;
   if(idx >= 1000){
      break;
   }
    }

    fclose(file);

    while (wait(NULL) > 0);

    return EXIT_SUCCESS;
}