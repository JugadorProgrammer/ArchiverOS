#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define ARGV_SIZE 3
#define HEADER_SIZE 1024
#define MAX_FILENAME_LENGTH 256

const char* delemiter = "\r\n";

const char* plus(const char* string1, const char* string2)
{
    size_t size1 = sizeof(string1);
    size_t size2 = sizeof(string2);

    char* result = malloc(size1 + size2);

    int i = 0;
    for(; i < size1; ++i)
    {
        result[i] = string1[i];
    }

    for(int j = 0; i < size1; ++i, ++j)
    {
        result[i] = string2[j];
    }

    return result;
}

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

void archive_directory(const char *directory_path, FILE *archive, const char* prev_dir)
{
    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", directory_path);
        return;
    }

    size_t bytes_count;
    char* buffer = NULL;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0
            || strcmp(entry->d_name, "..") == 0)
        {
            continue; // Пропускаем текущую и родительскую директории
        }

        printf("%d - %s [%d] %d\n",
               entry->d_ino, entry->d_name, entry->d_type, entry->d_reclen);

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

            char* temp = (prev_dir ? plus(prev_dir, entry->d_name): entry->d_name);

            archive_directory(filepath, archive, temp);
        }
        else if (S_ISREG(statbuf.st_mode))
        {
            FILE* file = fopen(filepath, "rb");
            long length = get_data_file_length(file);
            buffer = malloc(length);

            fread(buffer, 1, length, file);
            // write delemiter to file
            fwrite(delemiter, sizeof(delemiter), 1, archive);

            // write count of file's data
            // fwrite(length, sizeof(delemiter), 1, archive);

            char* file_path = plus((prev_dir ? plus(prev_dir, entry->d_name): entry->d_name), entry->d_name);
            //fwrite(archive, ,);
            // write file path
            //fwrite(filepath, 1, sizeof(filepath), archive);
            fwrite(file_path, 1, sizeof(file_path), archive);
            // write count of file's data
            fprintf(archive, "%ld", length);

            while ((bytes_count = fread(buffer, 1, sizeof(buffer), file)) > 0)
            {
                if (fwrite(buffer, 1, bytes_count, archive) != bytes_count)
                {
                    fprintf(stderr, "Error writing to output file"); //ошибка записи в выходной
                    break;
                }
            }

            free(buffer);
            fclose(file);
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

    archive_directory(directory_path, archive, NULL);
    //fprintf(stdout, "Ok");
    return 0;
}
