#include "ackerman.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "my_allocator.h"
#include <ctype.h>

int main(int argc, char ** argv) {
	
	//printf("test: %s %s", argv[0], argv[1]);
	
	int opt;
    unsigned int basic_block_size = 128, memory_length = 524288;


    while ((opt = getopt(argc, argv, "b:s:")) != -1)
        switch (opt)
        {
        case 'b':
              basic_block_size = atoi(optarg);
			  
            break;
        case 's':
               memory_length = atoi(optarg);
			   
            break;
        default:
			printf("bbs: %i", basic_block_size);
            abort ();
        }
		
	printf("bbs: %i", basic_block_size);	
	printf("mem: %i", memory_length);
  

  init_allocator(basic_block_size, memory_length);

  ackerman_main(); // this is the full-fledged test. 
  // The result of this function can be found from the ackerman wiki page or https://www.wolframalpha.com/. If you are not getting correct results, that means that your allocator is not working correctly. In addition, the results should be repetable - running ackerman (3, 5) should always give you the same correct result. If it does not, there must be some memory leakage in the allocator that needs fixing
  
  // please make sure to run small test cases first before unleashing ackerman. One example would be running the following: "print_allocator (); x = my_malloc (1); my_free(x); print_allocator();" the first and the last print should be identical.
  
  release_allocator();
}
