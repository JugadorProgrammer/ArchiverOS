#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define ARGV_SIZE 3
#define MAX_FILENAME_LENGTH 256

const size_t size_of_char = sizeof(char);
const char data_delemiter = '*';
const char file_mode = 'f';
const char dir_mode = 'd';

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
    if (!dir)
    {
        printf("Failed to open directory: %s\n", directory_path);
        return;
    }

    char* buffer = NULL;
    struct dirent *entry;

    while ((entry = readdir(dir)))
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
            //write mode
            fprintf(archive, "%c", dir_mode);
            // write length
            fprintf(archive, "%ld", strlen(entry->d_name));

            //wirte delemiter
            fprintf(archive, "%c", data_delemiter);
            // write directory name
            fwrite(entry->d_name, size_of_char, strlen(entry->d_name)/ size_of_char, archive);

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
            //write mode
            fprintf(archive, "%c", file_mode);

            // write length of file name
            fprintf(archive, "%ld", short_file_path);
            printf("File: %s\n", filepath + directory_path_length);

            //wirte delemiter
            fprintf(archive, "%c", data_delemiter);

            // write file name
            fwrite(filepath + directory_path_length, size_of_char, short_file_path / size_of_char, archive);
            // write lenth of file's data
            fprintf(archive, "%ld", length);

            //wirte delemiter
            fprintf(archive, "%c", data_delemiter);

            // write data to archive
            fwrite(buffer, size_of_char, length, archive);

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
    fclose(archive);
    return 0;
}
