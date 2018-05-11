#include <stdlib.h>
#include <stdio.h>
#include <CUnit/Basic.h>
#include "../libfractal/fractal.h"
#include "queue.h"

void testGetName(void){
    const char *name = "projet";
	int width = 4;
	int height = 2;
	double a = 0.5;
	double b = 0.4;
	struct fractal *f = fractal_new(name, width, height, a, b);
	CU_ASSERT_STRING_EQUAL(fractal_get_name(f), name);
	fractal_free(f);
}

int main(int argc, const char *argv[]){
    CU_pSuite pSuite = NULL;
    if(CUE_SUCCESS != CU_initialize_registry()){
    return CU_get_error();
    }

    if(NULL == pSuite){
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(NULL == CU_add_test(pSuite, "testGetName", getName) //||
       //NULL == CU_add_test(pSuite, "testGetName", getName) 
      // NULL == CU_add_test(pSuite, "testGetName", getName) 
       //NULL == CU_add_test(pSuite, "testGetName", getName) 
       //NULL == CU_add_test(pSuite, "testGetName", getName)
        )
       {
           CU_cleanup_registry();
           return CU_get_error();
       }

       CU_basic_set_mode(CU_BRM_VERBOSE);
       CU_basic_run_tests();
       CU_cleanup_registry();

       return CU_get_error();
}