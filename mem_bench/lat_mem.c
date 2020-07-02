#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
//#include "clock.h" /* routines to access the cycle counter */

#define MINBYTES (1 << 11)  /* Working set size ranges from 2 KB */
#define MAXBYTES (1 << 26)  /* ... up to 64 MB */
#define MAXSTRIDE 64        /* Strides range from 1 to 64 elems */
#define MAXELEMS MAXBYTES/sizeof(double) 

#define ASIZE (1 << 17)

int* data;      /* The global array we'll be traversing */

void init_data(int *data, int n);
void run_delay_testing();
double get_seque_access_result(int size, int stride, int type);
double get_random_access_result(int size, int type);

static struct timeval st;
static struct timeval ed;

static int test_type = 1;

int main(int ac, char* arv[])
{       
    init_data(data, MAXELEMS);  
    
    if(ac == 2){
    	test_type = atoi(arv[1]);
    }

    printf("Delay  (ns)\n");    
    run_delay_testing();
    printf("\n\n");
    
    exit(0);
}

void run_delay_testing(){   
    int size;        /* Working set size (in bytes) */
    int stride;      /* Stride (in array elements) */
    
    /*Print the size header */
    printf("\t");
    for (size = MAXBYTES; size >= MINBYTES; size >>= 1) {
        if (size > (1 << 20)){
            printf("%dm\t", size / (1 << 20));
        }else{
            printf("%dk\t", size / 1024);
        }
    }
    printf("\n");   

    /* estmate the band width sequencely */
    /*for (stride = 8; stride <= MAXSTRIDE; stride=stride+24) {
        printf("s%d\t", stride);        
        for (size = MAXBYTES; size >= MINBYTES; size >>= 1) {
	    if(test_type == 1)
            	printf("%.3f\t", get_seque_access_result(size, stride, test_type));
	    else
		printf("%.2f\t", get_seque_access_result(size, stride, test_type));
        }
        printf("\n");
    }*/
    
    /*  estmate the band width real randomly*/
    printf("random\t");
    /*for (size = MAXBYTES; size >= MINBYTES; size >>= 1) {
	if(1==test_type)
        	printf("%.3f\t", get_random_access_result(size,test_type));
	else
		printf("%.2f\t", get_random_access_result(size,test_type));
    }*/
    printf("%.3f\t", get_random_access_result(1024*1024*1024,test_type));
    printf("\n");
}


/* init_data - initializes the array */
void init_data(int *data, int n)
{
    int i;

    /*for (i = 0; i < n; i++)
    	data[i] = i;*/
    data = (int *)malloc(n*sizeof(int));
    memset(data, 0, n*sizeof(int));
}

void create_rand_array(int max, int count, int* pArr)
{
    int i;
    for (i = 0; i < count; i ++,pArr++) {
        int rd = rand();        
        int randRet = (long int)rd * max / RAND_MAX;
        *pArr = randRet;
    }
    return;
}



/*
 * seque_access 
 *      - access global data array step by step 
 */
long long seque_access(register int elems, register int stride) /* The test function */
{
    register int i;
    register long long result = 0; 
    volatile long long sink; 

    for (i = 0; i < elems; i += stride) {
    result += data[i];  
    }
    sink = result; /* So compiler doesn't optimize away the loop */
    return sink;
}


/*
 * random_access 
 *      - access global by random way indexed by random_index_arr 
 */
long long random_access(int* random_index_arr, register int count) /* The test function */
{ 
    register int i;
    register long long result = 0; 
    volatile long long sink; 

    for (i = 0; i < count; i++) {
        result += data[*(random_index_arr+i)];  
    }
    sink = result; /* So compiler doesn't optimize away the loop */
    return sink;
}


/*
 * get_random_access_result : test the computer storage(register,cache or memory) band width. 
 *      - size: data size will be tested
 *      - stride: steps
 *      - type: 0 get the band width, 
 *      -       1 get the consumed cpu cycles per operation.
 */
double get_seque_access_result(int size, int stride, int type)
{   
    register int i;  
    long int operations;
    long int total_accessed_bytes;
    long int used_microseconds;
    
    int samples = 3000;     
    int elems = size / sizeof(int); 
            
    /* run the test*/
    gettimeofday(&st, NULL);
    for(i=0; i<samples; i++){
        seque_access(elems, stride);
    }
    gettimeofday(&ed, NULL);
    used_microseconds = (ed.tv_sec - st.tv_sec)*1000000 + (ed.tv_usec - st.tv_usec);
    if(0==used_microseconds){
        return 0;
    }
    
    /* analysize result*/
    operations = (long int)samples * (long int)elems / stride;  
    total_accessed_bytes = operations * sizeof(int);

    
    double result = 0;
    if(0==type){ /* get width */
	double total_mb = total_accessed_bytes / (1024. * 1024.);
        result = total_mb / used_microseconds * 1000000;
        
    }else if(1==type){/* get cycles_per_operation */        
        result = (double)used_microseconds*1000/operations; 
    }   
    
    return result;
}
    
/*
 * get_random_access_result : test the computer storage(register,cache or memory) band width. 
 *      - size: data size will be tested
 *      - type: 0 get the band width, 
 *      -       1 get the consumed cpu cycles per operation.
 */
double get_random_access_result(int size, int type)
{   
    register int i;
    int *p;
    
    long int operations;
    long int total_accessed_bytes;
    long int used_microseconds;
    
    register int samples = 1;      
    int elems = size / sizeof(int); 
    int access_count = elems;
    
    /* prepare for random access*/
    int* random_access_arr = malloc(access_count*sizeof(int));
    memset(random_access_arr, 0, access_count*sizeof(int));  
    create_rand_array(elems, access_count, random_access_arr);  
    
        
    /* run the test*/
    gettimeofday(&st, NULL);
    for(i=0; i<samples; i++){
        random_access(random_access_arr, access_count);
    }
    gettimeofday(&ed, NULL);
    used_microseconds = (ed.tv_sec - st.tv_sec)*1000000 + (ed.tv_usec - st.tv_usec);

    /* analysize result*/
    operations = (long int)samples * (long int)access_count;    
    total_accessed_bytes = operations * sizeof(int);

    
    double result = 0;;
    if(0==type){ /* get width */
	double total_mb = total_accessed_bytes / (1024.*1024.);
        result = total_mb  / used_microseconds * 1000000;
        
    }else if(1==type){/* get cycles_per_operation */        
        result = used_microseconds*1000./operations; 
    }   
    
    return result;
}
