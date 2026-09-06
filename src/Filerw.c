#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool file_readable(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp != NULL) {
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

int read_values(int value_to_read) {
    int val;
    char buffer[256];
    FILE *fp;
    switch (value_to_read) {
        case 0:
            if (file_readable("data/hscore.txt") == true) {
                fp = fopen("data/hscore.txt", "r");
                fgets(buffer, sizeof(buffer), fp);
                val = atoi(buffer);
                fclose(fp);
                break;
            }
        case 1:
            if (file_readable("data/coins.txt") == true) {
                fp = fopen("data/coins.txt", "r");
                fgets(buffer, sizeof(buffer), fp);
                val = atoi(buffer);
                fclose(fp);
                break;
            }
        default:
            val = -1;
    }
    return val;
}

int write_values(int value_to_write, int act_value){
    FILE *fp;
    int val; 
    switch (value_to_write) {
        case 0:
            fp = fopen("data/hscore.txt", "w");
            fprintf(fp, "%d", act_value);
            val = 0;
            break;
        case 1:
            fp = fopen("data/coins.txt", "w");
            fprintf(fp, "%d", act_value);
            val = 0;
            break;
        default:
            val = -1;
    }
    return val;
}