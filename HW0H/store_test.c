/*
 * A program that illustrates one approach to writing a test suite for
 * testing the task_store() function.  It is only to help you get 
 * started so you should expand and customize this program.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "task.h"

/* 
 * Create an instance of the structures used to represent a task
 * They are made external so they are accessible to each test
 *
 */
  task my_task;
  VM my_vm;
  FS my_fs;
  paged my_paged;
  pinned my_pinned;

// organize as a suite of test functions each of which performs
// a specific set of tests
  void test1(void);
  void test_store(int);

void main (int argc, char *argv[])
{
  void *rc;

  printf("Test: Started\n");

  // initialize the pointers linking the individual structures
  // note that the individual test functions may overwrite
  // these values to create different test cases
  my_task.vm_ptr = &my_vm;
  my_task.fs_ptr = &my_fs;
  my_vm.paged_ptr = &my_paged;
  my_vm.pinned_ptr = &my_pinned;

  // be sure to initialize the task_store function
  rc = task_store(INIT, NULL, NULL);
  if (rc == NULL) {
    printf("Test: INIT failed\n");
    return;
  }
  else
    printf("Test: INIT successful\n");

  // invoke the first test function
  //test1();

  test_store(50);
  // more test functions called here

  rc = task_store(INIT, NULL, NULL);
  if (rc == NULL) {
    printf("Test: Init again successful (suppose to return NULL)\n");
    return;
  }
  else
    printf("Test: Init again failed\n");

  return;
}

/*
 * This function performs the most basic test which is storing 
 * a task representation and then using LOCATE to return and
 * check the value of each field in the stored copy.
 */
void test_store(int times){
  void* rc;
  int i;
  int id = 100;
  char buf[5];
  long pid = 1234;
  long inode_start = 1000;
  long inode_end = 1000000;
  int paged = 1024;
  int pinned = 2048;
  int size = 16;

  void** paged_start_storage = (void **)malloc(times * sizeof(void *));
  void** paged_end_storage = (void **)malloc(times * sizeof(void *)); 
  void** pinned_start_storage = (void **)malloc(times * sizeof(void *));
  void** pinned_end_storage = (void **)malloc(times * sizeof(void *));


  for(i = 0; i < times; i++){
    my_task.pid = pid;
    my_fs.inode_start = inode_start;
    my_fs.inode_end = inode_end;

    my_paged.paged_start = (void *)malloc(paged);
    my_paged.paged_end = (void *)malloc(size);

    my_pinned.pinned_start = (void *)malloc(pinned);
    my_pinned.pinned_end = (void *)malloc(size);

    snprintf(buf, sizeof(buf), "%d", id);
    rc = task_store(STORE, buf, &my_task);
    if (rc == NULL) {
      printf("Test Store %d: STORE failed\n", i);
      return;
    }
    else
      printf("Test Store %d: STORE successful\n", i);

    paged_start_storage[i] = my_paged.paged_start;
    paged_end_storage[i] = my_paged.paged_end;
    pinned_start_storage[i] = my_pinned.pinned_start;
    pinned_end_storage[i] = my_pinned.pinned_end;

    id++;
    pid++;
    inode_start++;
    inode_end++;
    paged += size;
    pinned += size;
  }

  char buf2[20];
  id = 100;
  pid = 1234;
  inode_start = 1000;
  inode_end = 1000000;
  paged = 1024;
  pinned = 2048;
  for(i = 0; i < times; i++){
    my_task.pid = pid;
    my_fs.inode_start = inode_start;
    my_fs.inode_end = inode_end;

    snprintf(buf2, sizeof(buf2), "%d pid", id);
    rc = task_store(LOCATE, buf2, NULL);
    if (rc == NULL) 
      printf("Test %d: LOCATE pid failed\n", id);
    else
      printf("Test %d: LOCATE pid %ld got %ld\n",id, pid, *(long *)rc);

    snprintf(buf2, sizeof(buf2), "%d paged_start", id);
    rc = task_store(LOCATE, buf2, NULL);
    if (rc == NULL) 
      printf("Test %d: LOCATE paged_start failed\n",id);
    else{
      if(paged_start_storage[i] == *(void **)rc){
        printf("Test %d: LOCATE paged_start successful\n",id);
      }else{
        printf("Test %d: LOCATE paged_start failed should be %p, while got %p\n", id, paged_start_storage[i], *(void **)rc);
      }
      //printf("Test %d: LOCATE paged_start %p got %p\n", id, paged_start_storage[i], *(void **)rc);
    }

    snprintf(buf2, sizeof(buf2), "%d paged_end", id);
    rc = task_store(LOCATE, buf2, NULL);
    if (rc == NULL) 
      printf("Test %d: LOCATE paged_end failed\n", id);
    else{
      if(paged_end_storage[i] == *(void **)rc){
        printf("Test %d: LOCATE paged_end successful\n",id);
      }else{
        printf("Test %d: LOCATE paged_end failed should be %p, while got %p\n", id, paged_end_storage[i], *(void **)rc);
      }
      //printf("Test %d: LOCATE paged_start %p got %p\n", id, paged_start_storage[i], *(void **)rc);
    }
    snprintf(buf2, sizeof(buf2), "%d pinned_start", id);    
    rc = task_store(LOCATE, buf2, NULL);
    if (rc == NULL) 
      printf("Test %d: LOCATE pinned_start failed\n", id);
    else{
      if(pinned_start_storage[i] == *(void **)rc){
        printf("Test %d: LOCATE pinned_start successful\n",id);
      }else{
        printf("Test %d: LOCATE pinned_start failed should be %p, while got %p\n", id, pinned_start_storage[i], *(void **)rc);
      }
      //printf("Test %d: LOCATE paged_start %p got %p\n", id, paged_start_storage[i], *(void **)rc);
    }
    snprintf(buf2, sizeof(buf2), "%d pinned_end", id);
    rc = task_store(LOCATE, buf2, NULL);
    if (rc == NULL) 
      printf("Test %d: LOCATE pinned_end failed\n", id);
    else{
      if(pinned_end_storage[i] == *(void **)rc){
        printf("Test %d: LOCATE pinned_end successful\n",id);
      }else{
        printf("Test %d: LOCATE pinned_end failed should be %p, while got %p\n", id, pinned_end_storage[i], *(void **)rc);
      }
      //printf("Test %d: LOCATE paged_start %p got %p\n", id, paged_start_storage[i], *(void **)rc);
    }
    snprintf(buf2, sizeof(buf2), "%d inode_start", id);
    rc = task_store(LOCATE, buf2, NULL);
    if (rc == NULL) 
      printf("Test %d: LOCATE inode_start failed\n", id);
    else
      printf("Test %d: LOCATE inode_start %ld got %ld\n", id ,inode_start, *(long *)rc);

    snprintf(buf2, sizeof(buf2), "%d inode_end", id);
    rc = task_store(LOCATE, buf2, NULL);
    if (rc == NULL) 
      printf("Test %d: LOCATE inode_end failed\n", id);
    else
      printf("Test %d: LOCATE inode_end %ld got %ld\n", id, inode_end, *(long *)rc);


    id++;
    pid++;
    inode_start++;
    inode_end++;
    paged += size;
    pinned += size;
  }

  free(paged_start_storage);
  free(paged_end_storage);
  free(pinned_start_storage);
  free(pinned_end_storage);
  return;

}
void test1(void)
{
  void *rc;

  // populate all the non-pointer fields
  // in this test, the initial pointer fields are used
  my_task.pid = 1234;

  my_fs.inode_start = 1000;
  my_fs.inode_end = 1000000;

  my_paged.paged_start = (void *)malloc(1024);
  my_paged.paged_end = (void *) malloc(16);

  my_pinned.pinned_start = (void *) malloc(2048);
  my_pinned.pinned_end = (void *) malloc(16);

  // store the task representation
  rc = task_store(STORE, "100", &my_task);
  if (rc == NULL) {
    printf("Test 1: STORE failed\n");
    return;
  }
  else
    printf("Test  1: STORE successful\n");

  // retrieve all the fields and compare with the original values
  rc = task_store(LOCATE, "100 pid", NULL);
  if (rc == NULL) 
     printf("Test 1: LOCATE pid failed\n");
  else
     printf("Test 1: LOCATE pid %ld got %ld\n", (long)1234, *(long *)rc);

  rc = task_store(LOCATE, "100 paged_start", NULL);
  if (rc == NULL) 
     printf("Test 1: LOCATE paged_start failed\n");
  else
     printf("Test 1: LOCATE paged_start %p got %p\n", my_paged.paged_start, *(void **)rc);

  rc = task_store(LOCATE, "100 paged_end", NULL);
  if (rc == NULL) 
    printf("Test 1: LOCATE paged_end failed\n");
  else
    printf("Test 1: LOCATE paged_end %p got %p\n", my_paged.paged_end, *(void **)rc);

  rc = task_store(LOCATE, "100 pinned_start", NULL);
  if (rc == NULL) 
    printf("Test 1: LOCATE pinned_start failed\n");
  else
    printf("Test 1: LOCATE pinned_start %p got %p\n", my_pinned.pinned_start, *(void **)rc);

  rc = task_store(LOCATE, "100 pinned_end", NULL);
  if (rc == NULL) 
    printf("Test 1: LOCATE pinned_end failed\n");
  else
    printf("Test 1: LOCATE pinned_end %p got %p\n", my_pinned.pinned_end, *(void **)rc);

  rc = task_store(LOCATE, "100 inode_start", NULL);
  if (rc == NULL) 
    printf("Test 1: LOCATE inode_start failed\n");
  else
    printf("Test 1: LOCATE inode_start %ld got %ld\n", (long)1000, *(long *)rc);

  rc = task_store(LOCATE, "100 inode_end", NULL);
  if (rc == NULL) 
    printf("Test 1: LOCATE inode_end failed\n");
  else
    printf("Test 1: LOCATE inode_end %ld got %ld\n", (long)1000000, *(long *)rc);

  return;
}

