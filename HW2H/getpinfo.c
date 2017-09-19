/*
 * Example kernel loadable module.  It illustrates the 
 * module infrastructure used in programming assignments
 * in the COMP 530H Lab.  The only function is to accept
 * an emulated "system call" to getpid from user space
 * and returns the character representation of the 
 * Linux process ID (pid) of the caller.
 */ 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/list.h>

#include "getpinfo.h" /* used by both kernel module and user program 
                     * to define shared parameters including the
                     * debugfs directory and file used for emulating
                     * a system call
                     */

/* The following two variables are global state shared between
 * the "call" and "return" functions.  They need to be protected
 * from re-entry caused by kernel preemption.
 */
/* The call_task variable is used to ensure that the result is
 * returned only to the process that made the call.  Only one
 * result can be pending for return at a time (any call entry 
 * while the variable is non-NULL is rejected).
 */

struct task_struct *call_task = NULL;
char *respbuf;  // points to memory allocated to return the result

int file_value;
struct dentry *dir, *file;  // used to set up debugfs file name

int getpinfo_single(struct task_struct *cur_task_ptr);

/* This function emulates the handling of a system call by
 * accessing the call string from the user program, executing
 * the requested function and preparing a response.
 *
 * This function is executed when a user program does a write()
 * to the debugfs file used for emulating a system call.  The
 * buf parameter points to a user space buffer, and count is a
 * maximum size of the buffer content.
 *
 * The user space program is blocked at the write() call until
 * this function returns.
 */

static ssize_t getpinfo_call(struct file *file, const char __user *buf,
                                size_t count, loff_t *ppos)
{
  int rc;
  char callbuf[MAX_CALL];  // local (kernel) space to store call string

  /*local variables for iterate over sibling linked-list*/
  struct task_struct *cur_task_ptr;
  struct list_head *pos;
  int num_head = 0;

  /* the user's write() call should not include a count that exceeds
   * the size of the module's buffer for the call string.
   */

  if(count >= MAX_CALL)
    return -EINVAL;  // return the invalid error code

  /* The preempt_disable() and preempt_enable() functions are used in the
   * kernel for preventing preemption.  They are used here to protect
   * state held in the call_task and respbuf variables
   */
  
  preempt_disable();  // prevents re-entry possible if one process 
                      // preempts another and it also calls this module

  if (call_task != NULL) { // a different process is expecting a return
     preempt_enable();  // must be enabled before return
     return -EAGAIN;
  }

  list_for_each(pos, &current->sibling){
    num_head++;
  }
  // allocate some kernel memory for the response
  respbuf = kmalloc(num_head * MAX_RESP, GFP_ATOMIC);
  if (respbuf == NULL) {  // always test if allocation failed
     preempt_enable(); 
     return -ENOSPC;
  }

  strcpy(respbuf,""); /* initialize buffer with null string */

  /* current is global for the kernel and contains a pointer to the
   * task_struct for the running process 
   */
  call_task = current;

  /* Use the kernel function to copy from user space to kernel space.
   */

  rc = copy_from_user(callbuf, buf, count);
  callbuf[MAX_CALL - 1] = '\0'; /* make sure it is a terminated string */

  if (strcmp(callbuf, "getpinfo") != 0) { // only valid call is "getpinfo"
      strcpy(respbuf, "Failed: invalid operation\n");
      printk(KERN_DEBUG "getpinfo: call %s will return %s\n", callbuf, respbuf);
      preempt_enable();
      return count;  /* write() calls return the number of bytes written */
  }

  if(call_task == NULL){
    //if call_task pointer is NULL
    strcpy(respbuf, "Failed: current process is null\n");
    printk(KERN_DEBUG "getpinfo: call %s will return %s\n", callbuf, respbuf);
    preempt_enable();
    return count;
  }

  if(call_task->real_parent == NULL){
    //if real_parent pointer is NULL
    strcpy(respbuf, "Failed: current's parent process is null\n");
    printk(KERN_DEBUG "getpinfo: call %s will return %s\n", callbuf, respbuf);
    preempt_enable();
    return count;
  }
  sprintf(respbuf, "Success:\n"); // start forming a response in the buffer

  /*TODO: get list head node from current process and iterating over the list*/
  /*current's real_parent's children list_head cannot be NULL because it is a struct*/

  list_for_each_entry(cur_task_ptr, &call_task->real_parent->children, sibling){
    int res;
    if((res = getpinfo_single(cur_task_ptr)) < 0){
      /*the getpinfo_single function returns error codes*/
      if(res == -1){
        /*if the current task pointer is NULL*/
        strcpy(respbuf, "Failed: invalid sibling task_struct\n");
      }else if(res == -2){
        /*if real_parent pointer is NULL*/
        strcpy(respbuf, "Failed: parent process is null\n");
      }else if(res == -3){
        /*if current process's mm pointer is NULL*/
        sprintf(respbuf, "Failed: pointer to process %d struct mm_struct is null\n", task_pid_nr(cur_task_ptr));
      }else{
        /*should not happen*/
        strcpy(respbuf, "Failed: unknown reason\n");
      }
      printk(KERN_DEBUG "getpinfo: call %s will return %s\n", callbuf, respbuf);
      preempt_enable();
      return count;
    }
  }

  /* Here the response has been generated and is ready for the user
   * program to access it by a read() call.
   */

  printk(KERN_DEBUG "getpinfo: call %s will return %s", callbuf, respbuf);
  preempt_enable();
  
  *ppos = 0;  /* reset the offset to zero */
  return count;  /* write() calls return the number of bytes */
}

/* This function emulates the return from a system call by returning
 * the response to the user as a character string.  It is executed 
 * when the user program does a read() to the debugfs file used for 
 * emulating a system call.  The buf parameter points to a user space 
 * buffer, and count is a maximum size of the buffer space. 
 * 
 * The user space program is blocked at the read() call until this 
 * function returns.
 */

static ssize_t getpinfo_return(struct file *file, char __user *userbuf,
                                size_t count, loff_t *ppos)
{
  int rc; 

  preempt_disable(); // protect static variables

  if (current != call_task) { // return response only to the process making
                              // the getpid request
     preempt_enable();
     return 0;  // a return of zero on a read indicates no data returned
  }

  rc = strlen(respbuf) + 1; /* length includes string termination */

  /* return at most the user specified length with a string 
   * termination as the last byte.  Use the kernel function to copy
   * from kernel space to user space.
   */

  /* Use the kernel function to copy from kernel space to user space.
   */
  if (count < rc) { // user's buffer is smaller than response string
    respbuf[count - 1] = '\0'; // truncate response string
    rc = copy_to_user(userbuf, respbuf, count); // count is returned in rc
  }
  else 
    rc = copy_to_user(userbuf, respbuf, rc); // rc is unchanged

  kfree(respbuf); // free allocated kernel space

  respbuf = NULL;
  call_task = NULL; // response returned so another request can be done

  preempt_enable(); // clear the disable flag

  *ppos = 0;  /* reset the offset to zero */
  return rc;  /* read() calls return the number of bytes */
} 

// Defines the functions in this module that are executed
// for user read() and write() calls to the debugfs file
static const struct file_operations my_fops = {
        .read = getpinfo_return,
        .write = getpinfo_call,
};

int getpinfo_single(struct task_struct *cur_task_ptr){
  char resp_line[MAX_LINE]; // local (kernel) space for a response

  /*char array for store command string*/
  char comm[MAX_COMM];

  /*variables for storing the info from struct task_struct*/
  pid_t cur_pid = 0;
  pid_t cur_parent_pid = 0;
  long cur_state = 0;
  unsigned int cur_flags = 0;
  int cur_normal_prio = 0;

  /*local variable for storing variables memory management structure*/
  int cur_map_count = 0;
  unsigned long cur_shared_vm = 0;
  unsigned long cur_exec_vm = 0;
  unsigned long cur_stack_vm = 0;
  unsigned long cur_total_vm = 0;

  int is_current = 0;

  /*check if the current task pointer is NULL*/
  if(cur_task_ptr == NULL){
    return -1;
  }
  /*Do stuff here to load all the info into local variables*/
  /* Use kernel functions for access to pid for a process 
  */
  cur_pid = task_pid_nr(cur_task_ptr);

  /*kernel functions for accesss command string of a process*/
  get_task_comm(comm, cur_task_ptr);

  /*kernel function to get parent's pid*/
  if(cur_task_ptr->real_parent == NULL){
    return -2;
  }
  cur_parent_pid = task_pid_nr(cur_task_ptr->real_parent);

  /*get current process's info in struct task_struct*/
  cur_state = cur_task_ptr->state;
  cur_flags = cur_task_ptr->flags;
  cur_normal_prio = cur_task_ptr->normal_prio;

  /*get current process's info in struct mm_struct
  if the process is not current running process
  before accessing mm_struct of other process do up_read function*/
    
  if(cur_pid == task_pid_nr(current)){
    is_current = 1;
  }

  if(cur_task_ptr->mm == NULL){
    return -3;
  }

  if(is_current == 0){
    down_read(&cur_task_ptr->mm->mmap_sem);
  }
    
    
  cur_map_count = cur_task_ptr->mm->map_count;
  cur_shared_vm = cur_task_ptr->mm->shared_vm;
  cur_exec_vm = cur_task_ptr->mm->exec_vm;
  cur_stack_vm = cur_task_ptr->mm->stack_vm;
  cur_total_vm = cur_task_ptr->mm->total_vm;

  /*finish accessing fields of mm_struct, release the semaphore*/
  if(is_current == 0){
    up_read(&cur_task_ptr->mm->mmap_sem);
  }
  /*form a response string line by line*/
  sprintf(resp_line, "     Current PID %d\n", cur_pid);
  strcat(respbuf, resp_line); //cur_pid
  sprintf(resp_line, "command %s\n", comm);
  strcat(respbuf, resp_line); //comm
  sprintf(resp_line, "parent PID %d\n", cur_parent_pid);
  strcat(respbuf, resp_line); //parent_pid
  sprintf(resp_line, "state %ld\n", cur_state);
  strcat(respbuf, resp_line); //state
  sprintf(resp_line, "flags 0x%08x\n", cur_flags);
  strcat(respbuf, resp_line); //flags
  sprintf(resp_line, "priority %d\n", cur_normal_prio);
  strcat(respbuf, resp_line); //normal_prio
  sprintf(resp_line, "VM areas %d\n", cur_map_count);
  strcat(respbuf, resp_line); //map_count
  sprintf(resp_line, "VM shared %lu\n", cur_shared_vm);
  strcat(respbuf, resp_line); //shared_vm
  sprintf(resp_line, "VM exec %lu\n", cur_exec_vm);
  strcat(respbuf, resp_line); //exec_vm
  sprintf(resp_line, "VM stack %lu\n", cur_stack_vm);
  strcat(respbuf, resp_line); //stack_vm
  sprintf(resp_line, "VM total %lu\n", cur_total_vm);
  strcat(respbuf, resp_line); //total_vm
  /*finish response*/
  return 1;

}
/* This function is called when the module is loaded into the kernel
 * with insmod.  It creates the directory and file in the debugfs
 * file system that will be used for communication between programs
 * in user space and the kernel module.
 */

static int __init getpinfo_module_init(void)
{

  /* create an in-memory directory to hold the file */

  dir = debugfs_create_dir(dir_name, NULL);
  if (dir == NULL) {
    printk(KERN_DEBUG "getpinfo: error creating %s directory\n", dir_name);
     return -ENODEV;
  }

  /* create the in-memory file used for communication;
   * make the permission read+write by "world"
   */

  file = debugfs_create_file(file_name, 0666, dir, &file_value, &my_fops);
  if (file == NULL) {
    printk(KERN_DEBUG "getpinfo: error creating %s file\n", file_name);
     return -ENODEV;
  }

  printk(KERN_DEBUG "getpinfo: created new debugfs directory and file\n");

  return 0;
}

/* This function is called when the module is removed from the kernel
 * with rmmod.  It cleans up by deleting the directory and file and
 * freeing any memory still allocated.
 */

static void __exit getpinfo_module_exit(void)
{
  debugfs_remove(file);
  debugfs_remove(dir);
  if (respbuf != NULL)
     kfree(respbuf);
}

/* Declarations required in building a module */

module_init(getpinfo_module_init);
module_exit(getpinfo_module_exit);
MODULE_LICENSE("GPL");
