#include "task.h"

//create a array of tasks with id structure with specified capacity
//a integer to track the current size
taskwid** tasks = NULL;
int curr_size = 0;

void *task_store(enum operation op, char *parm, task *ptr){
	if(op == INIT){
		//init operation
		if(tasks != NULL){
			return NULL;
		}
		//initilize persistent array with MAXSIZE
		//initilize curr_size as 0
		tasks = (taskwid **)malloc(MAXSIZE * sizeof(taskwid));
		curr_size = 0;

		//comment out the free part
		// for(int i = 0; i < curr_size; i++){
		// 	free(tasks[i]->t->fs_ptr);
		// 	free(tasks[i]->t->vm_ptr->pinned_ptr);
		// 	free(tasks[i]->t->vm_ptr->paged_ptr);
		// 	free(tasks[i]->t->vm_ptr);
		// 	free(tasks[i]->t);
		// 	free(tasks[i]);
		// }

		return (void *)1;
	}else if(op == STORE){
		//store operation
		if(tasks == NULL){
			//error: not initialized
			return NULL;
		}
		if(curr_size == MAXSIZE){
			//error: Max size exceeds
			return NULL;
		}
		//allocate memory for a structure I create to store numerical id and task struct
		taskwid* task_id = (taskwid *)malloc(sizeof(taskwid));
		task_id->t = (task *) malloc(sizeof(task));
		task_id->t->vm_ptr = (VM *) malloc(sizeof(VM));
		task_id->t->vm_ptr->paged_ptr = (paged *) malloc(sizeof(paged));
		task_id->t->vm_ptr->pinned_ptr = (pinned *)malloc(sizeof(pinned));
		task_id->t->fs_ptr = (FS *)malloc(sizeof(FS));

		int handle = sscanf(parm, "%lu", &task_id->id);//copy the id
		if(handle < 1){
			//error: invalid id
			return NULL;
		}
		//copy task information from pointer
		task_id->t->pid = ptr->pid;
		task_id->t->vm_ptr->paged_ptr->paged_start = ptr->vm_ptr->paged_ptr->paged_start;
		task_id->t->vm_ptr->paged_ptr->paged_end = ptr->vm_ptr->paged_ptr->paged_end;
		task_id->t->vm_ptr->pinned_ptr->pinned_start = ptr->vm_ptr->pinned_ptr->pinned_start;
		task_id->t->vm_ptr->pinned_ptr->pinned_end = ptr->vm_ptr->pinned_ptr->pinned_end;
		task_id->t->fs_ptr->inode_start = ptr->fs_ptr->inode_start;
		task_id->t->fs_ptr->inode_end = ptr->fs_ptr->inode_end;

		tasks[curr_size] = task_id;

		curr_size++;
		return (void *)1;

	}else{
		//locate operation
		if(tasks == NULL){
			//error: not initialized 
			return NULL;
		}
		char* id_ch = (char*)malloc(strlen(parm));
		char* v_ch = (char*)malloc(strlen(parm));
		int index = 0;
		while(parm[index] != ' '){
			if(index == strlen(parm)){
				//error: there is no space
				return NULL;
			}
			id_ch[index] = parm[index];
			index++;
		}
		//add a null terminator to the end if id char array
		id_ch[index] = '\0';
		index++;
		int offset = index;
		//loop until the end of string
		while(parm[index] != '\0'){
			v_ch[index - offset] = parm[index];
			index++;
		}
		//add a null terminator to the char array
		v_ch[index] = '\0';
		int id;
		int scan_res = sscanf(id_ch, "%d", &id);
		if(scan_res < 1){
			//error invalid id
			return NULL;
		}
		int i = 0;
		int found = 0;
		//look for task with the same id
		for(i = 0; i < curr_size; i++){
			if(id == tasks[i]->id){
				found = 1;
				break;
			}
		}
		if(found == 0){
			//error: requested id task no found
			return NULL;
		}
		//determine which field to return
		if(strcmp(v_ch, "pid") == 0){
			return &tasks[i]->t->pid;
		}else if(strcmp(v_ch, "inode_start") == 0){
			return &tasks[i]->t->fs_ptr->inode_start;
		}else if(strcmp(v_ch, "inode_end") == 0){
			return &tasks[i]->t->fs_ptr->inode_end;
		}else if(strcmp(v_ch, "paged_start") == 0){
			return &tasks[i]->t->vm_ptr->paged_ptr->paged_start;
		}else if(strcmp(v_ch, "paged_end") == 0){
			return &tasks[i]->t->vm_ptr->paged_ptr->paged_end;
		}else if(strcmp(v_ch, "pinned_start") == 0){
			return &tasks[i]->t->vm_ptr->pinned_ptr->pinned_start;
		}else if(strcmp(v_ch, "pinned_end") == 0){//pinned_end
			return &tasks[i]->t->vm_ptr->pinned_ptr->pinned_end;
		}else{
			//error: invalid value name
			return NULL;
		}

	}
}











