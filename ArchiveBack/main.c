#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#define ARGV_SIZE 3
#define MAX_FILENAME_LENGTH 256

const size_t size_of_char = sizeof(char);
// const char data_delemiter = '/';
const char file_mode = 'f';
const char dir_mode = 'd';

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

    int name_size;
    long size;
    long file_data_size;
    char mode;
    char data_delemiter;
    closedir(dir);

    while(1)
    {
        // read mode
        mode = fgetc(archive);
        //read size
        name_size = fscanf(archive, "%ld", &size);
        printf("NAME_SIZE: %d\n", name_size);
        if(name_size == -1)
        {
            return;
        }

        char *buffer = malloc(size);
        // skip delemiter
        data_delemiter = fgetc(archive);

        if(fread(buffer, size_of_char, size, archive) != size)
        {
            printf("Erorr read from file!\n");
            return;
        }

        printf("Buffer: %s\n", buffer);
        char* full_path = concatanation(directory_path, buffer);

        if (mode == file_mode)
        {
            FILE *file = fopen(full_path, "wb");

            if (!file)
            {
                printf("Failed to create file: %s\n", full_path);
                free(full_path);
                free(buffer);
                return;
            }

            free(buffer);

            // size data
            fscanf(archive, "%ld", &file_data_size);
            buffer = malloc(file_data_size);

            // skip delemiter
            data_delemiter = fgetc(archive);

            // read data from file
            fread(buffer, size_of_char, file_data_size, archive);
            // write data to file
            fwrite(buffer, size_of_char, file_data_size, file);
            fclose(file);
        }
        else if(mode == dir_mode)
        {
            if(!mkdir(full_path, 0777))
            {
                printf("Make dir: %s\n", full_path);
            }
            else
            {
                printf("Filed to make dir: %s\n", full_path);
                size = 0; // for exit
            }
        }
        else
        {
            printf("Incorrect mode: %s", mode);
            return;
        }

        free(full_path);
        free(buffer);
    }
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
