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

#include "PulseRainUARTConsole.h"

PulseRainUARTConsole_command_t PulseRainUARTConsole :: _commands[PULSERAIN_UART_CONSOLE_MAX_NUM_OF_COMMANDS];
int                            PulseRainUARTConsole :: _num_of_commands;        

void PulseRainUARTConsole::setPrompt(const char prompt_text[])
{
    int i;

    if (prompt_text) {     
      for (i = 0; i < (PULSERAIN_UART_CONSOLE_MAX_PROMPT_LENGTH - 1); ++i) {
         if (prompt_text[i] != '\0') {
              _prompt[i] = prompt_text[i];
         } else {
              break;
         }
      } // End of for loop
    }
}


PulseRainUARTConsole :: PulseRainUARTConsole (const char prompt_text[])
{
    int i = 0;

    _num_of_commands = 0;
    _history_pointer = 0;
    _current_line_char_count = 0;
    
    for (i = 0; i < PULSERAIN_UART_CONSOLE_MAX_PROMPT_LENGTH; ++i) {
        _prompt[i] = 0;
    }

    setPrompt (prompt_text);
    set_validate_input_function (_validate_input);

    _escape_seq_len = 2;
    _escape_sequence [0] = '\033';
    _escape_sequence [1] = '\133';
    _escape_sequence [2] = 0;
    _escape_sequence [3] = 0;

    _echo_enable = 0;

    add_cmd ({"help", "To show the full list of commands", help});

}
void PulseRainUARTConsole :: _update_history ()
{
    int i, j;

    
    for (i = 0; i < PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH; ++i) {
        _history_buffer[_history_pointer][i] = _current_line[i];
    }
     
}

void PulseRainUARTConsole :: _update_current_line()
{
    int i;
   
    _display_prompt();
    _display_prompt();
    
    _current_line_char_count = PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH;
    for (i = 0; i < PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH; ++i) {

        _current_line[i] = 0;
        
        if (_history_buffer[_history_pointer][i]) {
            _current_line[i] = _history_buffer[_history_pointer][i];
            Serial.write (&_current_line[i], 1);
        } else {
            _current_line_char_count = i;
            break;
        }
    }
    
}

void PulseRainUARTConsole :: _move_history_pointer (uint8_t up0_down1)
{
    if (up0_down1 == 0) {
        if (_history_pointer < (PULSERAIN_UART_CONSOLE_HISTORY_BUFFER_SIZE - 1)) {
            ++_history_pointer;
        } else {
            _history_pointer = 0;
        }
    } else {
        if (_history_pointer == 0) {
            _history_pointer = PULSERAIN_UART_CONSOLE_HISTORY_BUFFER_SIZE - 1;
        } else {
            --_history_pointer;
        }
    }
}

void PulseRainUARTConsole :: _display_prompt ()
{
        int i;
       
        for (i = 0; i < (PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH * 2 + PULSERAIN_UART_CONSOLE_MAX_PROMPT_LENGTH) ; ++i) {
            Serial.print ("\b");  
        }

        for (i = 0; i < (PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH + PULSERAIN_UART_CONSOLE_MAX_PROMPT_LENGTH); ++i) {
            Serial.print (" ");  
        }

        for (i = 0; i < (PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH * 2 + PULSERAIN_UART_CONSOLE_MAX_PROMPT_LENGTH); ++i) {
            Serial.print ("\b");  
        }

        Serial.print(_prompt);
}

void PulseRainUARTConsole :: set_validate_input_function (PulseRainUARTConsoleValidate_t func_ptr)
{
    _validate_input_func = func_ptr;
}

void PulseRainUARTConsole :: set_escape_sequence (const char esc_seq[], const int esc_seq_len)
{
    int i;

    _escape_seq_len = (esc_seq_len > PULSERAIN_UART_CONSOLE_MAX_ESCAPE_SEQ_LEN) ? PULSERAIN_UART_CONSOLE_MAX_ESCAPE_SEQ_LEN : esc_seq_len;

    for (i = 0; i < _escape_seq_len; ++i) {
        _escape_sequence [i]  = esc_seq[i];
    }
    
}

void PulseRainUARTConsole :: _execute_cmd ()
{
      int i;
      int found = 0;
      int cmd_index = 0;
      char *p;

      int argc;
      char* argv[PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH];

      if (strlen(_current_line) == 0) {
        Serial.print ("\n");
        return;
      }
     
      p = strtok (_current_line, " ");

      argc = 0;
      if (p) {
         
          argc = 1;
          argv[0] = p;

          for (i = 0; i < _num_of_commands; ++i) {
              if (strcmp (p, _commands[i].cmd_name) == 0) {
                  found = 1;
                  cmd_index = i;
              }
          }
          
          do {
              p = strtok (NULL, " ");
  
              if (p) {
                  argv[argc] = p;
                  ++argc;      
              }
            
          } while (p);

          
      }

      Serial.print ("\n\r");
      
      if (!found) {
          Serial.print (" Unknown Command!");
      } else {
          (_commands[cmd_index].func_ptr(argc, argv));
      }
     Serial.print ("\n\n");
      
}

void PulseRainUARTConsole :: _tab_auto_completion()
{
    int match_cnt = 0;
    int old_match_cnt;
    size_t index = 0, k;
    int match_index;
    
    int i = 0, j;
    int mismatch_flag [PULSERAIN_UART_CONSOLE_MAX_NUM_OF_COMMANDS] = {0};
    

    for (i = 0; i < _current_line_char_count; ++i) {
        
        match_cnt = 0;
        for (j = 0; j < _num_of_commands; ++j) {
            if ((_current_line[i] == _commands[j].cmd_name[i]) && (!mismatch_flag[j])) {
                ++match_cnt;
                index = j; 
            } else {
                mismatch_flag[j] = 1;
            }
        }

        if (match_cnt == 0) {
            break;
        }
        
    } // End for loop i
    
    match_cnt = 0;
    for (j = 0; j < _num_of_commands; ++j) {
        if (!mismatch_flag[j]) {
            ++match_cnt;
            index = j;
        }
    }

    old_match_cnt = match_cnt;
    
    if (i && (i == _current_line_char_count)) {
        k = _current_line_char_count;
        
        do {

            match_cnt = 0;
            for (j = 0; j < _num_of_commands; ++j) {
                if ((k < strlen(_commands[j].cmd_name)) && (k < strlen(_commands[index].cmd_name)) && (!mismatch_flag[j])) {
                    if (_commands[j].cmd_name[k] == _commands[index].cmd_name[k]) {
                        ++match_cnt;
                    }
                }
            }

            if (match_cnt == old_match_cnt) {
                _current_line[_current_line_char_count++] = _commands[index].cmd_name[k];
                _current_line[_current_line_char_count] = '\0';
                Serial.write (&_commands[index].cmd_name[k], 1);
                ++k;
            }
        } while (match_cnt == old_match_cnt);
    }

}

void PulseRainUARTConsole :: run ()
{ 
    uint8_t c;

    int esc_seq_detected = 0;

    _current_line_char_count  = 0;

    Serial.print("\n");
    _display_prompt();
    
    do {
        while (!Serial.available());
  
        c = Serial.read();

        if (_echo_enable) {
            Serial.write(&c, 1);    
        } else { // handle echo by cli
            
            if (esc_seq_detected < _escape_seq_len) { // in the middle of escape sequence detection
                
                if (c == _escape_sequence[esc_seq_detected]) { // continue to match escape sequence
                  Serial.write (&c, 1);
                  ++esc_seq_detected;
                  continue;
                } else {
                  esc_seq_detected = 0;
                }
            } else if (_escape_seq_len) { // control character
                esc_seq_detected = 0;

                switch (c) { 
                    case '\101' : // Up Arrow
                    
                        _move_history_pointer (0);
                        _update_current_line();
                      
                        break;
                        
                    case '\102' : // Down Arrow
                    
                        _move_history_pointer (1);
                        _update_current_line();
                        
                        break;
                        
                    default : 
                        break;
                }

                continue;
            
            }

            if ((c == '\010') || (c == '\177')) { // backspace or delete key
                if (_current_line_char_count > 0) {
                    --_current_line_char_count;
                    _current_line [_current_line_char_count] = 0;
                    Serial.print ("\b \b");
                }
            } else if (c == '\t') {
              
              _tab_auto_completion();
              
            } else if (_validate_input_func(c) && (_current_line_char_count < (PULSERAIN_UART_CONSOLE_MAX_INPUT_LENGTH - 1))) {
                Serial.write(&c, 1);
                _current_line [_current_line_char_count++] = c;
                _current_line [_current_line_char_count] = 0;
            } else if (c == '\r') {
                Serial.print ("\n");
                _current_line [_current_line_char_count] = 0;
                _update_history();
                _move_history_pointer (1);
                _execute_cmd ();
                _display_prompt ();
                _current_line_char_count = 0;
            }
        }
      
    } while (1);   
    
}

int PulseRainUARTConsole :: add_cmd (const PulseRainUARTConsole_command_t new_cmd)
{
    if (_num_of_commands < PULSERAIN_UART_CONSOLE_MAX_NUM_OF_COMMANDS) {
        _commands[_num_of_commands].cmd_name = new_cmd.cmd_name;
        _commands[_num_of_commands].cmd_help = new_cmd.cmd_help;
        _commands[_num_of_commands].func_ptr = new_cmd.func_ptr;
        ++_num_of_commands;
        return 0;
    } else {
        return -1;  
    }
}

void PulseRainUARTConsole :: set_echo_enable (const int echo_enable)
{
    _echo_enable = echo_enable;
}

PulseRainUARTConsole PULSERAIN_UART_CONSOLE("PulseRain >> ");
