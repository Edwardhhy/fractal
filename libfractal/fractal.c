#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
struct fractal *fractal = (struct fractal *) malloc(sizeof(struct fractal));
struct 
fractal->name=name;
fractal->width=width;
fractal->height=height;
fractal->a=a;
fractal->b=b;
}

void fractal_free(struct fractal *f)
{
    if(f==NULL){
        return;
    }
    free(f);
    f=NULL;
}

const char *fractal_get_name(const struct fractal *f)
{
    return f->name;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    return f->values[x][y];
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    f->values[x][y]=value;
}

int fractal_get_width(const struct fractal *f)
{
    return f->width;
}

int fractal_get_height(const struct fractal *f)
{
    return f->height;
}

double fractal_get_a(const struct fractal *f)
{
    return f->a;
}

double fractal_get_b(const struct fractal *f)
{
    return f->b;
}
