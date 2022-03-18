#include <stdio.h>
#include <stdlib.h>

struct data
{
    int age;
    int num;
};
int main()
{
    struct data *info;
    // info->age = 1;
    // info->num = 2;
    // struct data exp = {1, 2};
    // info = &exp;
    info = (struct data *)malloc(sizeof(struct data));
    info->age = 10;
    printf("%p  %d  %d\n", info, info->age, info->num);
    free(info);
}