#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#define ARGV_SIZE 3
#define MAX_FILENAME_LENGTH 256

const size_t size_of_char = sizeof(char);

void print_permissions(mode_t mode)
{
    printf("Permissions: ");
    printf((S_ISDIR(mode)) ? "d" : "-"); // is dir
    printf((mode & S_IRUSR) ? "r" : "-"); // owner read
    printf((mode & S_IWUSR) ? "w" : "-"); // owner write
    printf((mode & S_IXUSR) ? "x" : "-"); // owner execute
    printf((mode & S_IRGRP) ? "r" : "-"); // group read
    printf((mode & S_IWGRP) ? "w" : "-"); // group write
    printf((mode & S_IXGRP) ? "x" : "-"); // group execute
    printf((mode & S_IROTH) ? "r" : "-"); // others read
    printf((mode & S_IWOTH) ? "w" : "-"); // others write
    printf((mode & S_IXOTH) ? "x" : "-"); // others execute
    printf(" ");
}

char* concatanation(char *s1, char *s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    char *result = malloc(len1 + len2 + 1);

    if (!result)
    {
        fprintf(stderr, "malloc() failed: insufficient memory!\n");
        return NULL;
    }

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);

    return result;
}

void archive_directory_back(const char *directory_path, FILE *archive)
{
    DIR *dir = opendir(directory_path);
    if (!dir)
    {
        printf("Failed to open directory: %s\n", directory_path);
        return;
    }
    closedir(dir);

    int mode_size;
    long size;
    mode_t mode;
    char data_delemiter;
    do
    {
        // read mode
        mode_size = fscanf(archive, "%o", &mode);
        if(mode_size == -1)
        {
            continue;
        }
        print_permissions(mode);
        // skip delemiter
        data_delemiter = fgetc(archive);
        fscanf(archive, "%ld", &size);

        char *buffer = malloc(size);
        // skip delemiter
        data_delemiter = fgetc(archive);

        if(fread(buffer, size_of_char, size, archive) != size)
        {
            printf("Erorr read from file!\n");
            return;
        }

        // cut buffer string
        buffer[size] = '\0';
        char* full_path = concatanation(directory_path, buffer);

        if (!S_ISDIR(mode))
        {
            FILE* file = fopen(full_path, "wb");
            if (!file)
            {
                printf("Failed to open file: %s\n", full_path);
                free(full_path);
                free(buffer);
                return;
            }

            printf("File: %s ", buffer);
            // defining a file descriptor associated with a data stream
            int fp = fileno(file);

            if(fchmod(fp, mode))
            {
                printf("Failed to set permissions file: %s", full_path);
                fclose(file);
                free(buffer);
                free(full_path);
                return;
            }
            free(buffer);

            // size data
            fscanf(archive, "%ld", &size);
            buffer = malloc(size);
            printf("SIZE: %d\n", size);

            // skip delemiter
            data_delemiter = fgetc(archive);
            // read data from file
            fread(buffer, size_of_char, size, archive);

            // write data to file
            fwrite(buffer, size_of_char, size, file);
            fclose(file);
        }
        else
        {
            if(!mkdir(full_path, mode))
            {
                printf("Directory : %s\n", full_path);
            }
            else
            {
                printf("Filed to make dir: %s\n", full_path);
                free(full_path);
                free(buffer);
                return;
            }
        }

        free(full_path);
        free(buffer);
    }while(mode_size != -1);
}

int main(int argc, char **argv)
{
    if (argc != ARGV_SIZE)
    {
        printf("Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *archive_path = argv[1];
    const char *directory_path = argv[2];

    FILE *archive = fopen(archive_path, "rb");
    if (!archive)
    {
        printf("Failed to open archive");
        return EXIT_FAILURE;
    }

    archive_directory_back(directory_path, archive);
    fclose(archive);
    return 0;
}
