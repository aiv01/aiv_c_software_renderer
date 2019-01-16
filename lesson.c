#include <stdlib.h>
#include <stdio.h>

int *array_of_vector = NULL;
size_t array_of_vector_size = 0;

void append_vector(int value)
{
    array_of_vector_size++;
    int *resized_area = (int *)realloc(array_of_vector, sizeof(int) * array_of_vector_size);
    if (!resized_area)
    {
        // panic
        return;
    }

    array_of_vector = resized_area;
    array_of_vector[array_of_vector_size-1] = value;
}

char* read_file(const char *filename, size_t *file_size)
{
    FILE *fhandle = fopen(filename, "rb");
    if (!fhandle)
        return NULL;

    fseek(fhandle, 0, SEEK_END);
    *file_size = ftell(fhandle);
    fseek(fhandle, 0, SEEK_SET);

    char *data = malloc(*file_size);
    if (!data)
    {
        fclose(fhandle);
        return NULL;
    }

    fread(data, 1, *file_size, fhandle);
    fclose(fhandle);

    return data;
}