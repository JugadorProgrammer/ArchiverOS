#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define ARGV_SIZE 3
#define MAX_FILENAME_LENGTH 256

const size_t size_of_char = sizeof(char);
const char data_delemiter = '*';

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

        print_permissions(statbuf.st_mode);
        // write mode
        fprintf(archive, "%o", statbuf.st_mode);
        //wirte delemiter
        fprintf(archive, "%c", data_delemiter);

        if (S_ISDIR(statbuf.st_mode))
        {
            printf("Directory: %s\n", entry->d_name);
            // write length
            fprintf(archive, "%ld", strlen(entry->d_name));

            //wirte delemiter
            fprintf(archive, "%c", data_delemiter);
            // write directory name
            fwrite(entry->d_name, size_of_char, strlen(entry->d_name) / size_of_char, archive);

            archive_directory(filepath, archive, directory_path_length);
        }
        else if (S_ISREG(statbuf.st_mode))
        {
            FILE* file = fopen(filepath, "rb");
            long short_file_path = strlen(filepath + directory_path_length + 1);
            buffer = malloc(statbuf.st_size);

            // read data from file
            fread(buffer, 1, statbuf.st_size, file);

            // write length of file name
            fprintf(archive, "%ld", short_file_path);
            printf("File: %s SIZE: %d\n", filepath + directory_path_length, statbuf.st_size);

            //wirte delemiter
            fprintf(archive, "%c", data_delemiter);

            // write file name
            fwrite(filepath + directory_path_length + 1, size_of_char, short_file_path / size_of_char, archive);
            // write lenth of file's data
            fprintf(archive, "%ld", statbuf.st_size);

            //wirte delemiter
            fprintf(archive, "%c", data_delemiter);

            // write data to archive
            fwrite(buffer, size_of_char, statbuf.st_size, archive);

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
