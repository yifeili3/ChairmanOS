/*
 * source: http://www.osdever.net/bkerndev/Docs/keyboard.htm
 */

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"

#define LENGTH 128
#define KEYBOARD_IRQ 1
#define PORT 0x60
#define L    0x26
#define ESC  0x01
#define lower_upper_case_diff 32
#define num_terminal 3
#define VIDEO 0xB8000
#define ATTRIB 0x7
#define IS_DISP_TERMINAL 1
#define NOT_DISP_TERMINAL 0

#define NUM_COLS 80
#define NUM_ROWS 25
#define CTRL_PRESSED    0x1D
#define CTRL_RELEASED   0x9D
#define LEFT_SHIFT_PRESSED   0x2A
#define LEFT_SHIFT_RELEASED  0xAA
#define RIGHT_SHIFT_PRESSED  0x36
#define RIGHT_SHIFT_RELEASED 0xB6
#define ALT_PRESSED   0x38 
#define ALT_RELEASED  0xB8
#define CAPS_LOCK_PRESSED   0x3A
#define LEFT_ARROW_PRESSED  0x4B
#define RIGHT_ARROW_PRESSED 0x4D
#define UP_ARROW_PRESSED    0x48
#define DOWN_ARROW_PRESSED  0x50
#define BACKSPACE_PRESSED   0x0E
#define ENTER_PRESSED 0x1C
#define LOWER_BYTE 0xFF
#define FIRST_BIT 0x80
#define ASCII_LOWER_Z 122
#define ASCII_LOWER_A 97
#define ASCII_UPPER_Z 90
#define ASCII_UPPER_A 65
#define NOT_PRESSED 0
#define PRESSED 1
#define S_PRESSED 0x9F

// terminal_read_flag: 1 for readable, 0 for still typing
volatile int terminal_read_flag[3];
unsigned char keyboard_buffer[INPUT_SIZE]; //buffer to store keyboard input
unsigned char terminal_buffer[INPUT_SIZE]; // when hitting enter, fill terminal buffer with keyboard buffer

// the current displayed terminal index
uint8_t disp_terminal = 0;

// flag for the speical keys
int kbd_shift;
int kbd_ctrl;
int kbd_alt;
int kbd_caps_lock; 

// special flag to check whether there is a terminal initializing
int8_t flag_initialize_terminal;

// read and print the keyboard input, dealing with special cases at the same time
static void printable_char_generate(char intr);

// dealing with switch terminal
void switch_terminal(int32_t new_terminalID);

// keyboard lower case
unsigned char kbdus[LENGTH] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t',     /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,      /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`',   0,   /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',     /* 49 */
    'm', ',', '.', '/',   0,        /* Right shift */
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};
//Keycode array
// keyboard upper case
unsigned char kbdus_shift[LENGTH] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
    '(', ')', '_', '+', '\b', /* Backspace */
    '\t',     /* Tab */
    'Q', 'W', 'E', 'R', /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0,      /* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
    '\"', '~',   0,   /* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
    'M', '<', '>', '?',   0,        /* Right shift */
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};  

/* void handle_keyboard_interrupt()
 * INPUT: NONE
 * OUTPUT: NONE
 * FUNCTION: the handler function to handle keyboard interrupts
 */
void handle_keyboard_interrupt(){
  //  get the input buffer
  char buffer = inb(PORT);
  
  // call main function
  printable_char_generate(buffer);

  // send eoi to irq
  send_eoi(KEYBOARD_IRQ);
}

/* static void printable_char_generate
 * INPUT: NONE
 * OUTPUT: NONE
 * FUNCTION: the main function to handle keyboard interrupts
 */
static
void printable_char_generate(char intr){
  // get input buffer
  uint8_t buffer = (uint8_t) intr;

  // check whether input is special key
  if(buffer == CTRL_PRESSED){
    //ctrl pressed
    kbd_ctrl = 1;
    return;
  }

  if((buffer & LOWER_BYTE) == CTRL_RELEASED){
    //ctrl released
    kbd_ctrl = 0;
    return;
  }

  if(buffer == ALT_PRESSED){
    //alt pressed
    kbd_alt = 1;
    return;
  }

  if((buffer & LOWER_BYTE) == ALT_RELEASED){
    //alt released
    kbd_alt = 0;
    return;
  }

  if(buffer == CAPS_LOCK_PRESSED){
    kbd_caps_lock = (kbd_caps_lock + 1)%2;
    //caps_lock status
    return;
  }

  if(buffer == RIGHT_SHIFT_PRESSED || buffer == LEFT_SHIFT_PRESSED){
    //shift pressed
    kbd_shift = 1;
    return;
  }

  if((buffer & LOWER_BYTE) == RIGHT_SHIFT_RELEASED || (buffer & LOWER_BYTE) == LEFT_SHIFT_RELEASED){
    //shift released
    kbd_shift = 0;
    return;
  }

  // Ctrl + L: clear screen
  if(buffer == L && kbd_ctrl == 1){
    clear();
    terminals[disp_terminal].input_count=0;
    return;
  }

  // Esc: halt
  if(buffer == ESC){
    send_eoi(KEYBOARD_IRQ);
    int i;
    for(i=0;i<INPUT_SIZE;i++){
      terminal_buffer[i]='\0';
      keyboard_buffer[i]='\0';
    }
    sys_halt(0);
    terminals[disp_terminal].input_count=0;
    return;
  }

  // backspace pressed
  if(buffer == BACKSPACE_PRESSED){
    //if there are characters in buffer, then enable backspace
   if(terminals[disp_terminal].input_count>0){
      terminals[disp_terminal].input_count--; //change index in keyboard_buffer
      keyboard_buffer[terminals[disp_terminal].input_count]='\0';
      //call backspace helper func to clear video memory
      backspace();
    }
    return;
  }

  // pass buffer to terminal read
  if(buffer == ENTER_PRESSED){  

    // finish up the line  
    if(terminals[disp_terminal].input_count<INPUT_SIZE){
      keyboard_buffer[terminals[disp_terminal].input_count] ='\n';
    }
     //flag to determine whether user pressess enter
      terminal_read_flag[disp_terminal] = 1;
      // if the read func haven't finished reading, stay

      int i=0;
      // copy to terminal buffer
      for(i=0;i < terminals[disp_terminal].input_count;i++)
        terminal_buffer[i]=keyboard_buffer[i]; 
      
      //clear buffer
      for(i=0;i<INPUT_SIZE;i++)
        keyboard_buffer[i]='\0'; 
      
      //clear buffer count
      if((terminals[disp_terminal].input_count !=0 && screen_x[disp_terminal] == 0 && screen_y[disp_terminal] != (NUM_COLS + 1)))
        screen_y[disp_terminal]--;

      putc(kbdus[ENTER_PRESSED], IS_DISP_TERMINAL);
      terminals[disp_terminal].input_count = 0;
    return;
  }

/*    left/right arrow
    if(buffer == LEFT_ARROW_PRESSED || buffer == RIGHT_ARROW_PRESSED){
      if(buffer == LEFT_ARROW_PRESSED)
        //printf("LEFT arrow pressed!\n");
        //move_cursor(0);
      else{
       //printf("RIGHT arrow pressed!\n");
        //move_cursor(1);
    }

    //up/down arrow
    if(buffer == UP_ARROW_PRESSED || buffer == DOWN_ARROW_PRESSED){
      if(buffer == UP_ARROW_PRESSED)
        //printf("UP arrow pressed!\n");
      else
        //printf("DOWN arrow pressed!\n");

      return;
    }*/

  //switch terminal, need some way to get the current terminal
  if(kbd_alt == 1 && (buffer == 0x3B || buffer == 0x3C || buffer == 0x3D)){
    if(buffer == 0x3B)
      switch_terminal(0);
    else if(buffer == 0x3C)
      switch_terminal(1);
    else if(buffer == 0x3D)
      switch_terminal(2);
    return;
  }
  
  // it's release case or unsupported key, return
  if((((char)buffer) & FIRST_BIT) || (int)kbdus[(int) buffer] == 0)
    return;

  //handle normal case
  if(terminals[disp_terminal].input_count<INPUT_SIZE){

    // shift is not pressed
    if(kbd_shift == NOT_PRESSED){
      int c = (int)kbdus[(int) buffer];

      // caps_lock is pressed
      if(kbd_caps_lock == PRESSED){
        if(c >= ASCII_LOWER_A && c <= ASCII_LOWER_Z){ 

          //print out upper case
          keyboard_buffer[terminals[disp_terminal].input_count]= c-lower_upper_case_diff;
          terminals[disp_terminal].input_count++;
          printf("%c", (char) (c - lower_upper_case_diff));
          return;
        }

        keyboard_buffer[terminals[disp_terminal].input_count]= c; //print out lower case
        terminals[disp_terminal].input_count++;
        printf("%c", (char) c);

        return;
      }
      // caps_lock is not pressed
      keyboard_buffer[terminals[disp_terminal].input_count]= c; //print out lower case
      terminals[disp_terminal].input_count++;
      printf("%c", (char) c);
      return;
    }
    // shift is pressed
    else{
      int c = (int)kbdus_shift[(int) buffer];

      // caps_lock is pressed
      if(kbd_caps_lock == PRESSED){
        if(c >= ASCII_UPPER_A && c <= ASCII_UPPER_Z){    

          keyboard_buffer[terminals[disp_terminal].input_count]= c+lower_upper_case_diff;
          terminals[disp_terminal].input_count++;
          printf("%c", (char) (c + lower_upper_case_diff));
          return;
        }
			
        keyboard_buffer[terminals[disp_terminal].input_count]= c;
        terminals[disp_terminal].input_count++;
        printf("%c", (char) c);
        return;
      }

      // caps_lock is not pressed
      keyboard_buffer[terminals[disp_terminal].input_count]= c;
      terminals[disp_terminal].input_count++;
      printf("%c", (char) c);
      return;
    }
  }
}

/**
 * DESCRIPTION: terminal open
 * INPUT: const uin8_t * filename
 * OUTPUT: NONE 
 * RETURN:  -1 for error, 0 for success opening the file 
 * SIDEEFFECTS: NONE
 */ 
int32_t terminal_open(const uint8_t* filename){
    return 0;
}


/**
 * DESCRIPTION: terminal close
 * INPUT: int32_t * fd  file descriptor 
 * OUTPUT: NONE 
 * RETURN:  -1 for error, 0 for success closing the file 
 * SIDEEFFECTS: NONE
 */ 
int32_t terminal_close(int32_t fd){
  return 0;
}



/**
 * DESCRIPTION: reads data from the keyboard, a file, device(RTC), or directory.
 * INPUT: int32_t fd    - file descriptor
 *      void * buf    - pointer to the buffer
 *      int32_t nbytes  - number of bytes to write
 * OUTPUT: NONE
 * RETURN:  the number of bytes read, 0 if the initial file position is at or beyond the end of file
 * SIDEEFFECTS: NONE
 */ 
int32_t terminal_read (int32_t fd, void * buf, int32_t nbytes)
{
  //printf("terminal read\n");
  char* buf_i = (char*)buf;

  int i =0, bytesRead = 0, limit = LENGTH;
  //senity check
  if(nbytes<=0){
    return -1;
  }
  //update the bound, which limit is min(nbytes,LENGTH)
  if(nbytes < LENGTH) limit = nbytes;

  // if not readable, stay
  while(terminal_read_flag[scheduled_terminal] == 0);
  // check number limit, enter and end
  while(i < limit){
    if (terminal_buffer[i] == '\0') break;
    buf_i[i] = terminal_buffer[i];
    bytesRead++;
    i++;
  }
  buf_i[i] = '\n';
  // if(bytesRead==128)
  //   buf_i[LENGTH]='\n';
  //terminate buffer
  terminal_buffer[LENGTH-1]= '\0';
  //set flag to 0
  terminal_read_flag[scheduled_terminal] = 0;
  //clear terminal_buffer
  for(i=0;i<INPUT_SIZE;i++)
      terminal_buffer[i]='\0';
  return bytesRead+1;

}


/**
 * DESCRIPTION: writes data to terminal or to a device(RTC). in the case of terminal, all data
 *        are displayed immediately. For RTC, it sets the interrupt rate in Hz and rate
 *        of periodic interrupts accordingly. Writes to fules should always fail.
 * INPUT: int32_t fd    - file descriptor
 *      void * buf    - the buffer that hold the string for use
 *      int32_t nbytes  - number of bytes to be written
 * OUTPUT: keyboard input
 * RETURN:  the number of bytes written, 0 on success,  -1 on failure
 * SIDEEFFECTS: NONE
 */ 
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes)
{
  //return (keyboard_write(fd, (char*)buf, nbytes));
  char* buf_i = (char*)buf; 
  int i = 0, bytesWritten = 0; //num = nbytes
  if(nbytes<=0 || buf==NULL){  //senity check
    return -1;
  }
  cli();
  disable_irq(0);
  disable_irq(1);
  disable_irq(8);
  if(scheduled_terminal == disp_terminal){
    for(i = 0; i < nbytes; i++){
      if(buf_i[i] == '\0'){
        putc(' ', 1);
        continue;      
      }
      putc(buf_i[i], IS_DISP_TERMINAL);
      bytesWritten++;
    }    
  }
  else{
     for(i = 0; i < nbytes; i++){
      if(buf_i[i] == '\0'){
        putc(' ', 0);
        continue;      
      }
      putc(buf_i[i], NOT_DISP_TERMINAL);
      bytesWritten++;
    }
  }
  enable_irq(8);
  enable_irq(1);
  enable_irq(0);
  sti();

  return bytesWritten;
}

/**
 * DESCRIPTION: initiate the terminal.
 * INPUT:  NONE
 * OUTPUT: NONE
 * RETURN: NONE
 * SIDEEFFECTS: resets the cursor positions. clear buffer
 */ 
void terminal_init(){
  // clear the screen
  clear();

  // clear the buffer
  int i;
  int j;
  for(i=0;i<INPUT_SIZE;i++){
      keyboard_buffer[i]='\0'; 
      terminal_buffer[i]='\0';
  }

  // initialize terminal struct
  for (i = 0; i< 3; i++){
    terminals[i].shell_pid = -1;
    terminals[i].input_count = 0;
    for (j = 0; j < INPUT_SIZE; j++)
      terminals[i].local_keyboard_buffer[j] = '\0';
  }

  // set the first shell pid of the terminal
  // set the initialize flag
  terminals[0].shell_pid = 0;
  flag_initialize_terminal = 1;
}

/**
 * void switch_terminal(int32_t new_terminalID)
 * DESCRIPTION: initiate the terminal.
 * INPUT:  the new terminal index to display
 * OUTPUT: NONE
 * RETURN: NONE
 * SIDEEFFECTS: resets the cursor positions. reset the keyboard buffer
 */ 
void switch_terminal(int32_t new_terminalID){
  /* Map virtual address of new fish to video memory */
  vidmem_alloc(0xBA + 2*new_terminalID, 0xB8000);
  if(new_terminalID == disp_terminal)
    return;
  /* Map virtual address of old fish back to buffer memory */
  vidmem_alloc(0xBA + 2*disp_terminal, 0xBA000 + 2*4096*(disp_terminal));
  uint32_t i;
  for(i=0;i<INPUT_SIZE;i++)
      terminals[disp_terminal].local_keyboard_buffer[i]= keyboard_buffer[i];

  for(i=0;i<INPUT_SIZE;i++)
      keyboard_buffer[i] = terminals[new_terminalID].local_keyboard_buffer[i];

  cli();
  disable_irq(0);
  disable_irq(1);
  disable_irq(8);
  // copy the video memory to buffer
  memcpy((uint8_t*)(video_mem_phys_addr[disp_terminal]), (uint8_t*)0xB8000, 4000);

  // copy the video memory to buffer
  memcpy((uint8_t*)0xB8000, (uint8_t*)(video_mem_phys_addr[new_terminalID]), 4000);
  // set cursor position
  set_cursor(screen_y[new_terminalID], screen_x[new_terminalID]);
  // set current dispaly terminal id
  disp_terminal = new_terminalID;
  enable_irq(8);
  enable_irq(1);
  enable_irq(0);
  sti();
}

/**
 * uint8_t get_disp_terminal()
 * DESCRIPTION: get the index of current display terminal
 * INPUT:  NONE
 * OUTPUT: NONE
 * RETURN: the index of current display terminal
 * SIDEEFFECTS: NONE
 */ 
uint8_t get_disp_terminal(){
  return disp_terminal;
}

/**
 * uint8_t get_disp_terminal()
 * DESCRIPTION: get the flag of initialize terminal
 * INPUT:  NONE
 * OUTPUT: NONE
 * RETURN: the flag of initialize terminal
 * SIDEEFFECTS: NONE
 */ 
int8_t get_current_flag(){
  return flag_initialize_terminal;
}

/**
 * void set_current_flag(int8_t flag)
 * DESCRIPTION: set the flag of initialize terminal
 * INPUT:  the new flag of initialize terminal
 * OUTPUT: NONE
 * RETURN: NONE 
 * SIDEEFFECTS: change the status of the flag
 */ 
void set_current_flag(int8_t flag){
  flag_initialize_terminal = flag;
}


/**
 * void modify_terminal_curr_pid(int8_t pid)
 * DESCRIPTION: modify
 * INPUT:  the new pid for the current terminal
 * OUTPUT: NONE
 * RETURN: NONE 
 * SIDEEFFECTS: change the top pid of the current display terminal
 */ 
void modify_terminal_curr_pid(int8_t pid){
  terminals[disp_terminal].shell_pid = pid;
}

/**
 * void recover_terminal_curr_pid(int8_t current_pid, int new_terminal_pid)
 * DESCRIPTION: recover the top pid of the terminal
 * INPUT:  terminated pid and its parent
 * OUTPUT: NONE
 * RETURN: NONE 
 * SIDEEFFECTS: change the top pid of the terminal
 */ 
void recover_terminal_curr_pid(int8_t current_pid, int new_terminal_pid){
  // loop over the three termianls to look for the terminated pid, and recover it
  int i = 0;
  for(i=0; i<3; i++){
    if(terminals[i].shell_pid == current_pid){
      terminals[i].shell_pid = new_terminal_pid;
      break;
    }
  }
}
