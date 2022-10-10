#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define SUCCESS 1
#define ERROR 1
#define OPEN_DIR_ERROR -1

char* get_new_path(char* src_path, char* new_dir_name) {
    size_t src_path_len = strlen(src_path);
    size_t new_dir_len = strlen(new_dir_name);

    char* new_path = (char *) malloc((src_path_len + new_dir_len) * sizeof(char));
    
    for (int i = 1; i < src_path_len; ++i) {
        new_path[i] = src_path[i];
    }

    for (int i = src_path_len; i < src_path_len + new_dir_len; ++i) {
        new_path[i] = new_dir_name[i - src_path_len];
   }

    new_path[src_path_len + new_dir_len] = 0;

    return new_path; 
}

int copy_dir(char* src_path, char* dst_path) {
    DIR* cur_dir = opendir(src_path);
    if (cur_dir == NULL) {
        perror("opendir");
        return OPEN_DIR_ERROR;
    }

    struct dirent* entry = 
        (struct dirent *) malloc((sizeof(struct dirent) + pathconf(src_path, _PC_PATH_MAX) + 1));

    if (entry == NULL) {
        printf("bad alloc");
        return ERROR;
    }

    while (true) {
        int readdir_r_result = readdir_r(cur_dir, entry, &entry);

        if (readdir_r_result > 0) {
            printf("readdir_r error");
            free(entry);
            return readdir_r_result;
        }
        
        // EOF 
        if (entry == NULL) break;

        struct stat statbuf;
        int stat_result = stat(src_path, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {
            char* new_src_path = get_new_path(src_path, entry->d_name);
            char* new_dst_path = get_new_path(dst_path, entry->d_name);
            printf("%s\n", new_src_path);
            printf("%s\n", new_dst_path);

//            mkdir(new_src_path, __S_IFDIR); copy_dir(new_src_path, new_dst_path);
            free(new_src_path);
            free(new_dst_path);
        }

        if (S_ISREG(statbuf.st_mode)) { 
            printf("is reg\n");
        }

//        printf("%s\n", entry->d_name);
    }
}

int main(int argc, char* argv[]) {
    char* src_path = argv[1];
    char* dst_path = argv[2];

    int copy_result = copy_dir(src_path, dst_path);
    return 0;
}

