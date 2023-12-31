#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIRTUAL_MEMORY_SIZE 128
#define MAIN_MEMORY_SIZE 32
#define PAGE_SIZE 8
#define NUM_VIRTUAL_PAGES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)
#define NUM_MAIN_PAGES (MAIN_MEMORY_SIZE / PAGE_SIZE)

// Define a structure to represent a page
typedef struct {
    int validBit;
    int dirtyBit;
    int pageNumber;
    int fifo;
    int lru;
} Page;

Page ptable[NUM_VIRTUAL_PAGES];
int num_pages_used = -1;
int fifo_counter = 0; // the lower the number the longer it has been in main
int replacement_algorithim = 1; //1 for FIFO and 2 for LRU
int victim_page = 0; // 1 if there is a victim page

int lru_counter = 0; 

int findPage(int address){
    return (address/8);
}

int findAvailableMainMemoryPage(Page ptable[]){
    if (num_pages_used < NUM_MAIN_PAGES - 1){ // if there is available pages in main memory, insert into next
        return num_pages_used + 1;
    }
    else{
        victim_page = 1;
        if (replacement_algorithim == 1){ // FIFO Replacement
            int curr_longest_in_main_memory = 100000;
            int curr_longest_page;
            for (int i = 0; i < NUM_VIRTUAL_PAGES; i++) {
                if(ptable[i].validBit == 1){
                    if(ptable[i].fifo <= curr_longest_in_main_memory){
                        curr_longest_in_main_memory = ptable[i].fifo;
                        curr_longest_page = i;
                    }
                }
            }
            ptable[curr_longest_page].fifo = 100000;
            return ptable[curr_longest_page].pageNumber;
        } 
        else{
            //LRU Replacement
            int minLRU = 100000;
            int minFIFO = 100000;

            int curr_longest_page;
            for (int i = 0; i < NUM_VIRTUAL_PAGES; i++) {
                if(ptable[i].validBit == 1){ //if it is a page in main memory 
                    if(ptable[i].lru == minLRU){ //find the smallest page lru to evict 
                        //break tie by going with the one that was in the ptable first 
                        if(ptable[i].fifo <= minFIFO){
                            minFIFO = ptable[i].fifo; //sets to the smallest 
                            curr_longest_page = i;
                        }
                    }
                    else if(ptable[i].lru < minLRU){ //find the smallest page lru to evict 
                        minLRU = ptable[i].lru; //set to the smallest
                        curr_longest_page = i; //gets the smallest lru page 
                    }
                }
            }
            ptable[curr_longest_page].lru = 0; //reset lru to 0


            return curr_longest_page; //returns the smallest lru page 
        }
    }
}

//to find the main memory address that corresponds to the virtual memory address
int findMainMemoryAddress(int page_number, int virtual_address){
    return ((page_number * 8) + (virtual_address % 8));
}

int main(int argument, char* argv[]) {
    if (argument == 2) {
        char *algorithm = argv[1];
        if (strcmp(algorithm, "FIFO") == 0) { // Selected page replacement algorithm: FIFO
            replacement_algorithim = 1;
        } else if (strcmp(algorithm, "LRU") == 0){ // Selected page replacement algorithm: LRU
            replacement_algorithim = 2;
        }
    }

    int virtualMemory[VIRTUAL_MEMORY_SIZE]; //initalize virtual memory
    for (int i = 0; i < VIRTUAL_MEMORY_SIZE; i++) {
        virtualMemory[i] = -1;
    }
    int mainMemory[MAIN_MEMORY_SIZE]; //initalize main memory
    for (int j = 0; j < VIRTUAL_MEMORY_SIZE; j++) {
        mainMemory[j] = -1;
    }

    int address_counter = 0;
    for (int i = 0; i < NUM_VIRTUAL_PAGES; i++) {
        ptable[i].validBit = 0;
        ptable[i].dirtyBit = 0;
        ptable[i].pageNumber = i;
    }


    //3.2
    char input[128];

    while(1){
        printf("> ");
        
        fgets(input, 128, stdin);

        char *command = strtok(input, " \n");
        char *arg1 = NULL; 
        char *arg2 = NULL; 


        if (command != NULL){
            arg1 = strtok(NULL, " \n");        
        }

        if (arg1!= NULL){
            arg2 = strtok(NULL, " \n");        
        }

        if(strcmp(command,"quit") == 0){
            break;
        }
        else if (strcmp(command,"read") == 0 && arg1 != NULL){
            int virtual_address = atoi(arg1);

            int corresponding_page = findPage(virtual_address);
            
            if (ptable[corresponding_page].validBit == 0){
                printf("A Page Fault Has Occurred\n");
                printf("%d\n", virtualMemory[virtual_address]);

                int available_page = findAvailableMainMemoryPage(ptable);
                

                int temp_array[8] = {0}; 
                int curr_dirtyBit = 0;
                if (victim_page == 1){  
                    for(int i = 0; i < NUM_VIRTUAL_PAGES; i++){
                        if (ptable[i].pageNumber == available_page){
                            if (ptable[i].dirtyBit == 1){
                                curr_dirtyBit = 1;
                                break;
                            }
                        }
                    }
                    if (curr_dirtyBit == 1){ //if a page is evicted and dirtyBit is 1
                        int copy_back_to_virtual = available_page * 8;
                        for(int i = corresponding_page * 8; i < (corresponding_page * 8) + 8; i++){
                            temp_array[i] = virtualMemory[i];
                            virtualMemory[i] = mainMemory[copy_back_to_virtual]; //replacing page in virtual memory with evicted page thus replacing 8 addresses
                            copy_back_to_virtual++;
                        }
                        victim_page = 0;
                    }
                    ptable[available_page].validBit = 0; 
                    ptable[available_page].dirtyBit = 0;
                    ptable[available_page].pageNumber = available_page;
                    victim_page = 0;

                }


                int start_replacing_from = available_page * 8; //address to start replacing mainMemory data with virtualMemory data
                for(int i = corresponding_page * 8; i < (corresponding_page * 8) + 8; i++){
                    if(curr_dirtyBit == 1){
                        mainMemory[start_replacing_from] = temp_array[i];
                    }
                    else{
                        mainMemory[start_replacing_from] = virtualMemory[i]; //replacing the page thus replacing 8 addresses
                    }
                    start_replacing_from++;
                }
                ptable[corresponding_page].validBit = 1;
                ptable[corresponding_page].pageNumber = available_page;
                ptable[corresponding_page].fifo = fifo_counter;
                ptable[corresponding_page].lru = 0;

                num_pages_used++;
                fifo_counter++;

                

            } 
            else{
                int virtual_to_main = findMainMemoryAddress(ptable[corresponding_page].pageNumber, virtual_address);
                printf("%d\n", mainMemory[virtual_to_main]);
                ptable[corresponding_page].lru += 1; //increment lru when page read
            } 
        }
        else if (strcmp(command,"write") == 0 && arg1 != NULL && arg2 != NULL){
            int virtual_address = atoi(arg1);
            if (virtual_address <= VIRTUAL_MEMORY_SIZE ){
                int data = atoi(arg2);

                int corresponding_page = findPage(virtual_address);

                virtualMemory[virtual_address] = data; //writing to virtual memory

                if (ptable[corresponding_page].validBit == 0){
                    printf("A Page Fault Has Occurred\n");

                    int tempPage = findAvailableMainMemoryPage(ptable); //returns the page that should be evicted 
                    int available_page = tempPage;

                    if (num_pages_used == NUM_MAIN_PAGES-1){
                        available_page = ptable[tempPage].pageNumber;
                    }

                    int temp_array[8] = {0}; 
                    int curr_dirtyBit = 0;
                    if (victim_page == 1){  
                        for(int i = 0; i < NUM_VIRTUAL_PAGES; i++){
                            if (ptable[i].pageNumber == available_page){
                                if (ptable[i].dirtyBit == 1){
                                    curr_dirtyBit = 1;
                                    break;
                                }
                            }
                        }
                        if (curr_dirtyBit == 1){ //if a page is evicted and dirtyBit is 1
                            int copy_back_to_virtual = available_page * 8;
                            for(int i = corresponding_page * 8; i < (corresponding_page * 8) + 8; i++){
                                temp_array[i] = virtualMemory[i];
                                virtualMemory[i] = mainMemory[copy_back_to_virtual]; //replacing page in virtual memory with evicted page thus replacing 8 addresses
                                copy_back_to_virtual++;
                            }
                            victim_page = 0;
                        }
                        if (available_page != tempPage){
                            ptable[tempPage].pageNumber = tempPage;
                            ptable[tempPage].validBit = 0; 
                            ptable[tempPage].dirtyBit = 0;
                        }
                        else{
                            ptable[available_page].pageNumber = available_page;
                            ptable[available_page].validBit = 0; 
                            ptable[available_page].dirtyBit = 0;
                        }
                        victim_page = 0;

                    }


                    int start_replacing_from = available_page * 8; //address to start replacing mainMemory data with virtualMemory data
                    for(int i = corresponding_page * 8; i < (corresponding_page * 8) + 8; i++){
                        if(curr_dirtyBit == 1){
                            mainMemory[start_replacing_from] = temp_array[i];
                        }
                        else{
                            mainMemory[start_replacing_from] = virtualMemory[i]; //replacing the page thus replacing 8 addresses
                        }
                        start_replacing_from++;
                    }

                    if (ptable[corresponding_page].dirtyBit == 0){
                        ptable[corresponding_page].dirtyBit = 1;
                    }
                    ptable[corresponding_page].validBit = 1;
                    ptable[corresponding_page].pageNumber = available_page;
                    ptable[corresponding_page].fifo = fifo_counter;
                    ptable[corresponding_page].lru = 0;

                    num_pages_used++;
                    fifo_counter++;

                    

                } else{
                    //go to page in mainMemory through ptable[corresponding_page].pageNumber and then add the data there
                    if (ptable[corresponding_page].dirtyBit == 0){
                        ptable[corresponding_page].dirtyBit = 1;
                    }
                    int virtual_to_main = findMainMemoryAddress(ptable[corresponding_page].pageNumber, virtual_address);

                    mainMemory[virtual_to_main] = data;
                    ptable[corresponding_page].lru += 1; //increment lru when page written on 

                }
            }
        }
        else if (strcmp(command,"showptable") == 0){
            for(int i = 0; i < NUM_VIRTUAL_PAGES; i++){
                printf("%d:%d:%d:%d\n", i, ptable[i].validBit, ptable[i].dirtyBit, ptable[i].pageNumber);
            }

        }
        else if (strcmp(command,"showmain") == 0 && arg1 != NULL){
            int main_page = atoi(arg1);
            if (main_page < NUM_MAIN_PAGES){
                for(int i = main_page * 8; i < (main_page * 8) + 8; i++){
                    printf("%d: %d\n", i, mainMemory[i]);
                }
            }
        }
    }
    return 0;
}