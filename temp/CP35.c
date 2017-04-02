// in syscall.c
uint8_t new_pid_index;

MAXPROC = 6;
PID_t pidlist[MAXPROC];
void schedule(){

  // determine the next process from the queue
  uint8_t i = new_pid_index;
  while(1){
    i++;
    if(i == MAXPROC)
      i = 0;

    if(pidlist[i].flag == 1){
      // no need to context switch
      // child flag??
      if(i == new_pid_index)
        return;

      new_pid_index = i;
      break;
    }
  }

  // switch from old pid to new pid
  // TODO: context_switch & remapping video memory virtual address
  context_switch(current_pid, pidlist[new_pid_index].pid);
}

uint32_t get_current_pid(){
  return current_pid;
}

// in keyboard.c:
void switch_terminal(uint8_t new_terminalID){
  // save current keyboard buffer
  for(i=0;i<INPUT_SIZE;i++)
      terminals[currTerminal].local_keyboard_buffer[i]= keyboard_buffer[i];

  for(i=0;i<INPUT_SIZE;i++)
      keyboard_buffer[i] = terminals[new_pid].local_keyboard_buffer[i];

  currTerminal = new_pid;
  if(terminals[0].shell_pid == -1)
    sys_execute((uint8_t*) 'shell');

  set_cursor(terminals[new_pid].y, terminals[new_pid].x);
  // TODO: print keyboard buffer? Remapping video memory?
}

int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes)
{
  //return (keyboard_write(fd, (char*)buf, nbytes));
   char* buf_i = (char*)buf; 
   int i = 0, bytesWritten = 0; //num = nbytes
  if(nbytes<=0 || buf==NULL){  //senity check
    return -1;
  }

  // check if the process is foreground 
  if(check_process_foreground(get_current_pid()) == 1){
    for(i = 0; i < nbytes; i++){
      if(buf_i[i] == '\0')
        putc(' ');
      putc(buf_i[i]);
      bytesWritten++;
    }
  }
  else{
    // need to find which terminal the process belongs to and update screen position? 
    uint8_t terminal_id = ;
    uint32_t x = terminals[terminal_id].x;
    uint32_t y = terminals[terminal_id].y;
    for(i = 0; i < nbytes; i++){
      if(buf_i[i] == '\0')
        *(uint8_t *)(video_mem + ((NUM_COLS*y + x) << 1)) = ' ';
      else
        *(uint8_t *)(video_mem + ((NUM_COLS*y + x) << 1)) = buf_i[i];
      *(uint8_t *)(video_mem + ((NUM_COLS*y + x) << 1) + 1) = ATTRIB;
      //set_cursor(screen_y,screen_x);

      // update cursor position
      terminals[terminal_id].x++;
      if(terminals[terminal_id].x == 80){
        terminals[terminal_id].y++;
        terminals[terminal_id].x = 0;
      }

      if(terminals[terminal_id].y == 25){
        terminals[terminal_id].y = 24;
        scroll();
      }
      bytesWritten++;
    }
  }

  return bytesWritten;
}

void terminal_init(){
    clear();
    input_count=0;
    int i;
    int j;
    for(i=0;i<INPUT_SIZE;i++){
        keyboard_buffer[i]='\0'; 
        terminal_buffer[i]='\0';
    }

    for (i = 0; i< 3; i++){
      terminals[i].shell_pid = -1;
      for(j = 0; j<3; j++)
        terminals[i].pid_this_terminal[j] = -1;
      terminals[i].x = -1;
      terminals[i].y = -1;
      for (j = 0; j < INPUT_SIZE; j++)
        terminals[i].local_keyboard_buffer[j] = '\0';
    }
}

    // recoginize key combination
    if(kbd_alt == 1 && (buffer == 0x3B || buffer == 0x3C || buffer == 0x3D)){
      switch_terminal((buffer - 0x3B));
      return;
    }

// in keyboard.h:
typedef struct terminal {
  uint8_t  shell_pid;
  uint8_t  pid_this_terminal[3];
  uint32_t x;
  uint32_t y;
  uint8_t  local_keyboard_buffer[INPUT_SIZE];
} terminal_t;

static terminal_t terminals[3];
static int8_t currTerminal = 0;

// in kernel.c:
  /* Enable pit */
  enable_irq(0);

// TODO: context_switch function, paging?, how to initialize shells?(at boot time or by key?), video memory remapping,
// terminal write with background process(to check), switch terminal