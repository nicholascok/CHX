# CHX
A lightweight terminal hex editor.  
USAGE: `chx <filepath>`

## Default Implementations
  ### Instances
  CHX supports the ability to have several files open at once. Each open file is referred to as an instance. To open a new file within CHX use the interpreter command `open`, to close an instance, use the interpreter command `close`, and to switch to an instance, use the interpreter command `to` (see Interpreter Commands).
  ### Typing Modes
  | Mode          | Default Keybinding | Valid Characters | Description |
  |:-------------:|:------------------:|:----------------:|:------------|
  | **INSERT**        | I                  | Hex Characters   | characters are inserted into the file. by default, this will shift the rest of the file contents and change the filesize. (this can be disabled, if desired, in which case, inserting into the file results in data 'falling off' the end of the file, and deleting from the file results in data at the end of the file being filled with zeros) |
  | **ASCII INSERT**  | SHIFT + I          | All Characters   | characters are inserted into the file. Like with INSERT mode, this shifts the file contents and changes the filesize. However, unlike INSERT mode, this behaviour cannot be changed in the config. |
  | **REPLACE**       | R                  | Hex Characters   | characters overwrite file content, typing does not move the cursor. deleting from the file is the same as replacing with zeros. |
  | **ASCII REPLACE** | SHIFT + R          | All Characters   | characters overwrite file content, typing does not move the cursor. deleting from the file is the same as replacing with zeros. |
  | **TYPE**          | T                  | Hex Characters   | characters overwrite file content, typing moves the cursor. deleting from the file fills the void with zeros. |
  | **ASCII TYPE**    | SHIFT + T          | All Characters   | characters overwrite file content, typing moves the cursor. deleting from the file fills the void with zeros. |
  ### Interpreter Commands
  If a decimal number or hex offset (prefixed with 0x) is entered into the interpreter, the cursor jumps to that byte, otherwise the command is searched for in the lsit of interpreter commands, void first.
  | Command | Alias(es) | Description |
  |:--------|:--------|:------------|
  | `open <filepath>` | `o` | opens a new file and switches to the instance. |
  | `close <instance_num>` | `c` | closes the specified instance: if no instance is specified, the current instance is closed. |
  | `to <instance_num>` | none | switches to the specified instance. |
  | `find <string>` | `f`, `/` | moves the cursor to the next occurance of the specified string. |
  | `count <string>` | `cnt` | counts the number of occurances of the specified string in the file. |
  | `save <new_filename>` | `w` | saves the file under the specified filename: if no filename is specified, the filename is not changed. |
  | `se` | none | toggles between big and little endian interpretations of data. |
  | `ti` | none | toggles the visibility of the data inspector. |
  | `tp` | none | toggles the visibility of the ASCII preview. |
  | `exit` | `q!` | exits the program without saving. |
  | `quit` | `q` | if there are unsaved changes in the file, the user is prompted to save the current instance first, then exits the program. |
  | `wq` | none | saves the current instance and exits the program. |
  
  ### Editing the Layout of CHX
  There are a number of options that can be changed in config.h; many of htese options can also be changed live from within chx through the use of the `cfg` and `gcfg` interpreter commands. The `cfg` command affects the layout of the current instance and the `gcfg` command affects the layout of all currently open instances. The format of both commands is the same, and outlines as follows:  
  `cfg <property> <value>`  
  There are currently four properties that can be cahnged from within CHX:
  | Property Name | Reference | Default Value | Description|
  |:--------------|:----------|:-------------:|:-----------|
  | Row Number Length | `rnl` | 4 | changes the minimum row number length (in digits) |
  | Bytes per Row | `bpr` | 16 | changes the number of bytes displayed in each row |
  | Bytes in Group | `big` | 1 | changes the number of bytes displayed in each group |
  | Group Spacing | `gs` | 1 | changes the spacing between byte groups and layout features |
  
## Adding Keybinds
  Add keybinds to either the global or command mode array in config.h using the following format:  
  `[<key>] = <func_ptr>,`  
    
  You can create your own functions by adding them to either config.h or chx_defaults.c using the following format:  
  `void <my_func>(void) { /* code... */ }`
    
  Most keys are referenced by their literal value, for example, the A key is referenced by `'a'`
  and shift + A is referenced by `'A'`. In other cases, such as for the arrow keys, keys are referenced
  using a macro:
| Reference   | Key           |
| ----------- |:-------------:|
| KEY_UP      | up arrow      |
| KEY_DOWN    | down arrow    |
| KEY_RIGHT   | right arrow   |
| KEY_LEFT    | left arrow    |
| KEY_ESCAPE  | escape key    |
| KEY_PG_UP   | page up key   |
| KEY_PG_DN   | page down key |
| KEY_HOME    | home key      |
| KEY_END     | end key       |
| KEY_INSERT  | insert key    |
| KEY_DELETE  | delete key    |
| KEY_ENTER   | enter key     |
| KEY_TAB     | tab key       |
  
  For keys which are modified by alt or ctrl, put the key inside of the corresponding function. For example, alt + m is referenced by `CHX_ALT('m')`, 
  shift + alt + m is referenced by `CHX_ALT('M')`, and shift + delete is referenced by `CHX_SHIFT(KEY_DELETE)`.
  There are four of these functions:  
  - `CHX_CTRL(<key>)`
  - `CHX_CTRL_M(<key>)`
  - `CHX_ALT(<key>)`
  - `CHX_SHIFT(<key>)`
  
  Note that there is not currently support for multiple modifiers.
  
## Adding Interpreter Commands
  ### Void Interpreter Commands
  To add a void interpreter command, first create your function by adding it to either config.h or chx_defaults.c using the following format:  
  `void <my_func>(void) { /* code... */ }`  
  
  Then, add it to the list of void interpreter commands in config.h using the following syntax:  
  `(struct chx_void_command) {<func_ptr>, "<command_str>"},`  
  
  ### Interpreter Commands With Parameters
  To add an interpreter command with parameters, first create your function by adding it to either config.h or chx_defaults.c as follows:  
  `void <my_func>(char <num_params>, char** <param_list>) { /* code... */ }`  
  
  Then, add it to the list of interpreter commands in config.h using the following syntax:  
  `(struct chx_command) {<func_ptr>, "<command_str>"},`  
  
## Elclusion List
  By default, CHX stors the function pointer of your previous action (function called by key press or interpreter command entered).
  To exclude a function from being set as your last action, add it to the exclusion list in config.h.
