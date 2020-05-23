/*
###############################################################################
# Copyright (c) 2020, PulseRain Technology LLC 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License (LGPL) as 
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
# or FITNESS FOR A PARTICULAR PURPOSE.  
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
*/

#ifndef PULSERAIN_UART_CONSOLE
#define PULSERAIN_UART_CONSOLE

#include "Arduino.h"

using PulseRainUARTConsoleFunctions_t = int (*)(int argc, char* argv[]);
using PulseRainUARTConsoleValidate_t  = int (*)(char c);

// structure to add new command
struct PulseRainUARTConsole_command_t {
      const char *cmd_name;
      const char *cmd_help;
      PulseRainUARTConsoleFunctions_t func_ptr;
};

static_assert(sizeof(PulseRainUARTConsole_command_t)== 12);

class PulseRainUARTConsole {

    static constexpr int PULSERAIN_UART_CONSOLE_MAX_NUM_OF_COMMANDS = 32;
    static constexpr int PULSERAIN_UART_CONSOLE_MAX_PROMPT_LENGTH   = 16;
    static constexpr int PULSERAIN_UART_CONSOLE_MAX_ESCAPE_SEQ_LEN  = 4;
    static constexpr int PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH    = 64;
    static constexpr int PULSERAIN_UART_CONSOLE_HISTORY_BUFFER_SIZE = 8;

    public:
        PulseRainUARTConsole (const char prompt_text[]);

        void run();
        void setPrompt(const char prompt_text[]);
        
        int add_cmd (const PulseRainUARTConsole_command_t new_cmd);

        void set_validate_input_function (PulseRainUARTConsoleValidate_t func_ptr);
        void set_escape_sequence (const char esc_seq[], const int esc_seq_len);

        void set_echo_enable (const int echo_enable);

        static int help (int argc, char* argv[])
        {
              int i, j;
            
              int cur_index = 0, tmp_index = 0;
              char *tmp_cmd = (char*)"";
              char *cur_cmd = (char*)"";

              for (i = 0; i < _num_of_commands; ++i) {

                  tmp_cmd = (char*)"";
                  
                  for (j = 0; j < _num_of_commands; ++j) {
                     
                      if ((strcmp(_commands[j].cmd_name, cur_cmd) > 0)) {
                        
                          if ((strlen(tmp_cmd) == 0) || (strcmp(_commands[j].cmd_name, tmp_cmd) < 0)) {
                              tmp_index = j;
                              tmp_cmd = (char*)_commands[j].cmd_name;
                          }
                      }
                  } 
                      
                  cur_index = tmp_index;
                  cur_cmd = (char*)_commands[cur_index].cmd_name;
                      
                  
                  
                  Serial.print (i);
                  Serial.print (" ");
                  Serial.print (cur_cmd);
                  Serial.print (": ");
                  Serial.println ((char*)_commands[cur_index].cmd_help);
                  
              }

              return 0;
        }

        
    private:

        static int _validate_input (char c)
        {
            return (isAlphaNumeric(c) | isPunct(c) | isSpace(c));
        }

        void _display_prompt ();

        void _execute_cmd ();
        void _update_history ();
        void _move_history_pointer (uint8_t up0_down1);
        void _update_current_line();
        void _tab_auto_completion();

        int _echo_enable;
        static PulseRainUARTConsole_command_t _commands[PULSERAIN_UART_CONSOLE_MAX_NUM_OF_COMMANDS];
        PulseRainUARTConsoleValidate_t _validate_input_func;

        int  _history_pointer;
        static int  _num_of_commands;
        char _prompt[PULSERAIN_UART_CONSOLE_MAX_PROMPT_LENGTH];

        int  _escape_seq_len;
        char _escape_sequence[PULSERAIN_UART_CONSOLE_MAX_ESCAPE_SEQ_LEN];

        int  _current_line_char_count;
        char _current_line [PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH];
        char _history_buffer [PULSERAIN_UART_CONSOLE_HISTORY_BUFFER_SIZE][PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH];
};

#endif

