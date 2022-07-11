## Linux File Explorer


### Prerequisites
* System with g++ compiler installed `` sudo apt-get install g++``

### About the project
Building a fully functional file explorer application.
The application supports two modes:
* NORMAL MODE (default)
* COMMAND MODE

### Normal Mode
* Displays list of directories and files present in current folder.
* Informations displayed: file name, file size, ownership, last modified (in human readable format)
* Key presses and their functionalities:
    * `ENTER KEY`: When user presses it, the file/directory the cursor is at, opens.
    * `LEFT ARROW KEY`: Goes back to the previously visited directory.
    * `RIGHT ARROW KEY`: Goes to the next directory.
    * `BACKSPACE KEY`: Takes user up to one level.
    * `HOME KEY`: Takes user back to root directory, i.e., where the application was started.
    * `UP and DOWN keys`: used for moving cursor up and down to desired file or directory
#### Sample of Normal Mode
![Screenshot (439)](https://user-images.githubusercontent.com/72462189/178281852-c45a46c8-1feb-4b37-86e2-eacd1768a3d5.png)

### Command Mode
This mode is entered from Normal mode whenever `:` is pressed.
Paths starting with `~` are absolute paths with respect to the root, i.e., where the application was started.
Paths starting with `/` are relative paths with respect to the current directory.
##### Following commands are supported:
* `COPY`: copy <file_name(s)> <target_directory_path>
* `MOVE`: move <file_name(s)> <target_directory_path>
* `RENAME`: rename <old_file_name> <new_file_name>
* `CREATE FILE`: create_file <file_name> <destination_path>
* `CREATE DIRECTORY`: create_dir <diectory_name> <destination_path>
* `DELETE FILE`: delete_file <file_path>
* `DELETE DIRECTORY`: delete_dir <directory_path>
* `SEARCH`: search <file_name / directory_name>
* `GOTO`: goto <directory_path>
Pressing ESC KEY takes user back to Normal Mode.

#### Sample of Command Mode
![Screenshot (440)](https://user-images.githubusercontent.com/72462189/178282192-f2f1af38-49a7-4874-9307-b6660da8e5fd.png)

### How to execute the program-
* Open the terminal with root of the application folder as the present working directory
* Compile the cpp file: `g++ main.cpp`
* Run the executable file: `./a.out`
#### Language used-
C++
