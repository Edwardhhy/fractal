#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
struct fractal *fractal = (struct fractal *) malloc(sizeof(struct fractal));
// Problem allocating memory
if(fractal == NULL){
    return NULL;
}
fractal->value = (int *) malloc(sizeof(int)*((width*height)));
// Problem allocating memory
if(fractal->value == NULL){
    return NULL;
}
fractal->name=name;
fractal->width=width;
fractal->height=height;
fractal->a=a;
fractal->b=b;
return fractal;
}

void fractal_free(struct fractal *f)
{
    free(f);
    f=NULL;
}

const char *fractal_get_name(const struct fractal *f)
{
    return f->name;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    return *(f->value+x*f->width+y);
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    *(f->value+x*f->width+y) = val;
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
