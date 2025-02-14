#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void read_input(int *num_processes, int *num_resource_types, int **E, int ***C, int ***R, const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        printf("Error: Could not open file.\n");
        exit(1);
    }

    fscanf(file, "%d", num_processes);
    fscanf(file, "%d", num_resource_types);

    *E = (int *)malloc(*num_resource_types * sizeof(int));
    for (int i = 0; i < *num_resource_types; i++)
    {
        fscanf(file, "%d", &((*E)[i]));
    }

    *C = (int **)malloc(*num_processes * sizeof(int *));
    for (int i = 0; i < *num_processes; i++)
    {
        (*C)[i] = (int *)malloc(*num_resource_types * sizeof(int));
        for (int j = 0; j < *num_resource_types; j++)
        {
            fscanf(file, "%d", &((*C)[i][j]));
        }
    }

    *R = (int **)malloc(*num_processes * sizeof(int *));
    for (int i = 0; i < *num_processes; i++)
    {
        (*R)[i] = (int *)malloc(*num_resource_types * sizeof(int));
        for (int j = 0; j < *num_resource_types; j++)
        {
            fscanf(file, "%d", &((*R)[i][j]));
        }
    }

    fclose(file);
}

void detect_deadlock(int num_processes, int num_resource_types, int *E, int **C, int **R, int *deadlocked_processes, int *num_deadlocked)
{
    int *A = (int *)malloc(num_resource_types * sizeof(int));
    for (int i = 0; i < num_resource_types; i++)
    {
        A[i] = E[i];
        for (int j = 0; j < num_processes; j++)
        {
            A[i] -= C[j][i];
        }
    }

    int *finish = (int *)calloc(num_processes, sizeof(int));
    int *safe_sequence = (int *)malloc(num_processes * sizeof(int));
    int safe_seq_count = 0;

    while (1)
    {
        int found = 0;
        for (int i = 0; i < num_processes; i++)
        {
            if (!finish[i])
            { 
                int can_allocate = 1;
                for (int j = 0; j < num_resource_types; j++)
                {
                    if (R[i][j] > A[j])
                    {
                        can_allocate = 0;
                        break;
                    }
                }
                if (can_allocate)
                { 
                    for (int j = 0; j < num_resource_types; j++)
                    {
                        A[j] += C[i][j]; 
                    }
                    finish[i] = 1;                       
                    safe_sequence[safe_seq_count++] = i; 
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
            break; 
    }

    *num_deadlocked = 0;
    for (int i = 0; i < num_processes; i++)
    {
        if (!finish[i])
        { 
            deadlocked_processes[(*num_deadlocked)++] = i;
        }
    }

    free(A);
    free(finish);
    free(safe_sequence);
}

int main()
{
    const char *file_path = "input.txt";

    int num_processes, num_resource_types;
    int *E = NULL;
    int **C = NULL, **R = NULL;

    read_input(&num_processes, &num_resource_types, &E, &C, &R, file_path);

    int *deadlocked_processes = (int *)malloc(num_processes * sizeof(int));
    int num_deadlocked = 0;
    detect_deadlock(num_processes, num_resource_types, E, C, R, deadlocked_processes, &num_deadlocked);

    if (num_deadlocked == 0)
    {
        printf("No deadlock detected.\n");
    }
    else
    {
        printf("Deadlock detected! Deadlocked processes: ");
        for (int i = 0; i < num_deadlocked; i++)
        {
            printf("%d ", deadlocked_processes[i]);
        }
        printf("\n");
    }

    free(E);
    for (int i = 0; i < num_processes; i++)
    {
        free(C[i]);
        free(R[i]);
    }
    free(C);
    free(R);
    free(deadlocked_processes);

    return 0;
}
