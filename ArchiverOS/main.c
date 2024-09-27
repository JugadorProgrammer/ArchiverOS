#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define ARGV_SIZE 3
#define HEADER_SIZE 1024
#define MAX_FILENAME_LENGTH 256

const char* delemiter = "\r\n";

long get_data_file_length(FILE * file)
{
    if(!file)
    {
        return NULL;
    }

    long length;

    fseek (file, 0, SEEK_END);
    length = ftell (file);
    fseek (file, 0, SEEK_SET);

    return length;
}

void archive_directory(const char *directory_path, FILE *archive, long directory_path_length)
{
    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", directory_path);
        return;
    }

    char* buffer = NULL;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char filepath[MAX_FILENAME_LENGTH];
        snprintf(filepath, sizeof(filepath), "%s/%s", directory_path, entry->d_name);

        struct stat statbuf;
        if (stat(filepath, &statbuf) == -1)
        {
            fprintf(stderr, "Failed to get file status");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            printf("Start processing %s\n", entry->d_name);

            // write delimiter size
            fprintf(archive, "%ld", sizeof(delemiter));
            // write delemiter
            fwrite(delemiter, sizeof(delemiter), 1, archive);

            // write
            fprintf(archive, "%ld", strlen(entry->d_name));
            // write
            fwrite(entry->d_name, strlen(entry->d_name), 1, archive);

            archive_directory(filepath, archive, directory_path_length);
        }
        else if (S_ISREG(statbuf.st_mode))
        {
            FILE* file = fopen(filepath, "rb");
            long short_file_path = strlen(filepath + directory_path_length);
            long length = get_data_file_length(file);
            buffer = malloc(length);

            // read data from file
            fread(buffer, 1, length, file);
            // write delimiter size
            fprintf(archive, "%ld", sizeof(delemiter));
            // write delemiter
            fwrite(delemiter, sizeof(delemiter), 1, archive);
            // write file length
            fprintf(archive, "%ld", short_file_path);

            printf("File: %s\n", filepath + directory_path_length);

            // write file
            fwrite(filepath + directory_path_length, 1, short_file_path, archive);
            // write count of file's data
            fprintf(archive, "%ld", length);
            // write data to archive
            fwrite(buffer, 1, length, archive);

            fclose(file);
            free(buffer);
        }
    }

    closedir(dir);
}

int main(int argc, char **argv)
{
    if (argc != ARGV_SIZE)
    {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *directory_path = argv[1];
    const char *archive_path = argv[2];

    FILE *archive = fopen(archive_path, "wb");
    if (!archive)
    {
        perror("Failed to create archive");
        return EXIT_FAILURE;
    }

    archive_directory(directory_path, archive, strlen(directory_path));
    return 0;
}
