#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#define SUCCESS 0
#define ERROR 1
#define PTHREAD_CREATE_SUCCESS 0
#define STAT_ERROR -1
#define MKDIR_ERROR -1
#define READ_ERROR -1
#define WRITE_ERROR -1
#define REMOVE_ERROR -1
#define DELAY 1
#define DETACH_SUCCESS 0
#define OPEN_ERROR -1
#define BUF_SIZE 1000

typedef struct paths {
    char* src;
    char* dst;
} paths_t;

void cp_inner(paths_t* paths, DIR* cur_dir, DIR* dst_dir);
DIR* try_open_dir(char* path);

void free_paths(paths_t *paths) {
    free(paths->src);
    free(paths->dst);
    free(paths);
}

paths_t* make_paths(paths_t* other, char* additional_path) {
    paths_t* new_paths = (paths_t *) malloc(sizeof(paths_t));

    int additional_path_len = (additional_path == NULL ? 0 : strlen(additional_path) + 1);

    new_paths->src = 
        (char *) malloc((strlen(other->src) + additional_path_len + 1) * sizeof(char));
    new_paths->dst = 
        (char *) malloc((strlen(other->dst) + additional_path_len + 1) * sizeof(char));
    
    if (additional_path == NULL) {
        strcpy(new_paths->src, other->src);
        strcpy(new_paths->dst, other->dst);
        return new_paths;
    }

    strcat(strcat(strcpy(new_paths->src, other->src), "/"), additional_path);
    strcat(strcat(strcpy(new_paths->dst, other->dst), "/"), additional_path);
    return new_paths;
}

char is_cur_dir(char* name) {
    return name[0] == '.';
}

char is_up_dir(char* name) {
    return name[0] == '.' && name[1] == '.';
}

void* cp_dir(void* args) {
    paths_t* paths = (paths_t *) args;

    struct stat statbuf;

    int stat_result = stat(paths->src, &statbuf);

    if (stat_result == STAT_ERROR) {
        perror("stat");
        free(paths);
        
        return NULL;
    }

    int mkdir_res = mkdir(paths->dst, statbuf.st_mode);

    if (mkdir_res == MKDIR_ERROR && errno != EEXIST) {
        perror("mkdir");
        free(paths);
        return NULL;
    }

    DIR* src_dir = try_open_dir(paths->src);

    if (src_dir == NULL) {
        return NULL;
    }

    DIR* dst_dir = try_open_dir(paths->dst);

    if (paths->dst == NULL) {
        closedir(src_dir);
        return NULL;
    }

    cp_inner(paths, src_dir, dst_dir);

    closedir(src_dir);
    closedir(dst_dir);

    return NULL;
}

DIR* try_open_dir(char* path) {
    while (true) {
        DIR *dir = opendir(path);
        
        if (dir != NULL) {
            return dir;
        }

        if (errno != EMFILE) {
            perror(path);
            break;
        }

        struct timespec ts = {DELAY, 0};
        nanosleep(&ts, NULL);
    }
    return NULL;
}

int try_open_reg(char* path, int m, mode_t settings) {
    while (true) {
        int fd = open(path, m, &settings);

        if (fd != OPEN_ERROR) {
            return fd;
        }
        
        if (errno == EEXIST) {
            int remove_res = remove(path);

            if (remove_res == REMOVE_ERROR) {
                perror(path);
                break;
            }

            return try_open_reg(path, m, settings);
        }

        if (errno != EMFILE) {
            perror(path);
            break;
        }

        struct timespec ts = {DELAY, 0};
        nanosleep(&ts, NULL);
    }
    
    return OPEN_ERROR;
}

void copy_file_content(int src_fd, int dest_fd, paths_t *paths) {
    char buf[BUF_SIZE];
    while (true) {

        ssize_t bytes_read = read(src_fd, buf, BUF_SIZE);

        if (bytes_read == READ_ERROR) {
            perror(paths->src);
            return;
        }

        if (bytes_read == 0) {
            break;
        }

        ssize_t offset = 0;
        ssize_t bytes_written;
        while (offset < bytes_read) {
            bytes_written = write(dest_fd, buf + offset, bytes_read - offset);
            if (bytes_written == WRITE_ERROR) {
                perror(paths->dst);
                return;
            }
            
            offset += bytes_written;
        }
    }
}

void* cp_reg(void* args) {
    paths_t* paths = (paths_t *) args;

    struct stat statbuf;

    int stat_result = stat(paths->src, &statbuf);

    if (stat_result == STAT_ERROR) {
        perror("stat");
        free(paths);
        return NULL;
    }

    int src_fd = try_open_reg(paths->src, O_RDONLY, statbuf.st_mode);

    if (src_fd == OPEN_ERROR) {
        return NULL;
    }

    int dst_fd = try_open_reg(paths->dst, O_WRONLY | O_CREAT | O_EXCL, statbuf.st_mode);

    if (dst_fd == OPEN_ERROR) {
        close(src_fd);
        return NULL;
    }

    copy_file_content(src_fd, dst_fd, paths);

    close(src_fd);
    close(dst_fd);

    return NULL;
}

int execute_copy_routine(void *(*routine) (void *), void* args) {
    pthread_t thread;
    int create_result = pthread_create(&thread, NULL, routine, args);
    if (create_result != PTHREAD_CREATE_SUCCESS) {
        fprintf(stderr, "pthread create error\n");
        return ERROR;
    }

    int detach_res = pthread_detach(thread);

    if (detach_res != DETACH_SUCCESS) {
        return ERROR;
    }

    return SUCCESS;
}

void cp_inner(paths_t* paths, DIR* cur_dir, DIR* dst_dir) {
    int copy_res = SUCCESS;

    struct dirent* entry = 
        (struct dirent *) malloc((sizeof(struct dirent) + pathconf(paths->src, _PC_PATH_MAX) + 1));

    if (entry == NULL) {
        printf("bad alloc\n");
        return;
    }

    do {
        int readdir_r_result = readdir_r(cur_dir, entry, &entry);

        if (readdir_r_result > 0) {
            printf("readdir_r error");
            return;
        }
        
        // EOF 
        if (entry == NULL) break;

        struct stat statbuf;
        
        paths_t* new_paths = make_paths(paths, entry->d_name);

        int stat_result = stat(new_paths->src, &statbuf);

        if (stat_result == STAT_ERROR) {
            perror("stat");
            free(new_paths);
            break;
        }

        if (S_ISREG(statbuf.st_mode)) { 
            // printf("is reg\n");
            paths_t* new_paths_1 = make_paths(new_paths, NULL);
            
            copy_res = execute_copy_routine(cp_reg, (void *) new_paths_1);
        }

        else if (S_ISDIR(statbuf.st_mode)) {
            if (!is_cur_dir(entry->d_name) && !is_up_dir(entry->d_name)) {
                // printf("is dir\n");
                
                paths_t* new_paths_1 = make_paths(new_paths, NULL);

                copy_res = execute_copy_routine(cp_dir, (void *) new_paths_1);
            }
        }

        // else {
        //     // printf("not a dir or reg file\n");
        // }

        free(new_paths);
    } while (copy_res != ERROR);

    free(paths);
    free(entry);
}

int main(int argc, char** argv) {
    
    paths_t p = {argv[1], argv[2]};

    paths_t* paths = make_paths(&p, NULL);

    cp_dir((void *) paths);
    
    pthread_exit(NULL);
}