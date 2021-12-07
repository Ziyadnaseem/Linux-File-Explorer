#include <stdlib.h>
#include <termios.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#define limit 10
#define clr printf("\033[H\033[J")
#define clrline printf("\033[K")
using namespace std;
struct termios old_tio;
string rootpath;			//stores the root path, i.e., where the application was started
char path[1000];			//stores the current directory path
struct winsize w;
int top=0,bottom=top+limit,cursor=1,centre,col,total,command_pos,cursorIndex,statusline,flag=0;
vector<string>contents;
vector<string>commandArray;
stack<string>back;
stack<string>forw;

void cursor_pos(int ro,int co)			//sets cursor position
{
	cout<<"\033["<<ro<<";"<<co<<"H";
	fflush(stdout);
}

void enableRawMode()
{
	tcgetattr(STDIN_FILENO,&old_tio);
	struct termios new_tio=old_tio;
	new_tio.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO,TCSAFLUSH,&new_tio);
}

void disableRawMode()
{
	tcsetattr(STDIN_FILENO,TCSAFLUSH,&old_tio);
}

void initialize(char *path)					//initialize
{
	rootpath=string(path);
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
	command_pos=w.ws_row-5;
	centre=w.ws_col/2;
	statusline=command_pos+2;

}	

void showcurr()						//display files and directories in the current directory
{
	
	total=contents.size();
	struct stat info;
	clr;
	for(auto itr=top;itr<min(bottom,total);itr++)
	{	
	
		string mod_path=string(path)+"/"+string(contents[itr]);
		char*fpath=new char[mod_path.length()+1];
		strcpy(fpath,mod_path.c_str());
		lstat(fpath,&info);
		mode_t inf=info.st_mode;
		
        	int contentsize=info.st_size;
		if(contentsize/1048576 >1)
		{
			cout<<contentsize/1048576<<"MB";
		}
		else if(contentsize/1024 >1)
		{
			cout<<contentsize/1024<<"KB";
		}
		else
		{
			cout<<contentsize<<"B";
		}
        	       			
		
		cout<<"\t"; 

  		struct passwd* user_owner=getpwuid(info.st_uid);
		struct group* group_owner=getgrgid(info.st_gid);
		cout<<user_owner->pw_name<<","<<group_owner->gr_name<<",";
        	
        	
		((inf & S_IFDIR) ? cout<<"d" : cout<<"f");
		((inf & S_IRUSR) ? cout<<"r" : cout<<"-");
		((inf & S_IWUSR) ? cout<<"w" : cout<<"-");
		((inf & S_IXUSR) ? cout<<"x" : cout<<"-");
		((inf & S_IRGRP) ? cout<<"r" : cout<<"-");
		((inf & S_IWGRP) ? cout<<"w" : cout<<"-");
		((inf & S_IXGRP) ? cout<<"x" : cout<<"-");
		((inf & S_IROTH) ? cout<<"r" : cout<<"-");
		((inf & S_IWOTH) ? cout<<"w" : cout<<"-");
		((inf & S_IXOTH) ? cout<<"x" : cout<<"-");      	
        	
        	
		string mod_time=ctime(&info.st_mtime);
		mod_time.pop_back();
		cout<<"\t"<<mod_time;
		cout<<"\t";
		
		
		if((S_ISDIR(inf)))
		{
			cout<<"\033[1;36m"<<string(contents[itr])<<"\033[0m";
		}
		else
		{
			cout<<"\033[1;37m"<<string(contents[itr])<<"\033[0m";
		}
				
		cout<<endl;
        	
	}
	cursor_pos(cursor,0);
	return;
}

void readcurr(char const* dir_path)				//read current directory
{
	struct dirent* dir_obj;
	DIR* dir;
	if(!(dir=opendir(dir_path)))
	{
		cursor_pos(cursor,0);
		return;
	}
	chdir(dir_path);
	getcwd(path,1000);
	contents.clear();
	while(dir_obj=readdir(dir))
	{
		contents.push_back(dir_obj->d_name);
	}
	closedir(dir);
	top=0;
	total=contents.size();
	bottom=min(top+limit,total);
	cursor=1;
	showcurr();
	return;
}

void parent()
{
	if(path==rootpath)
	{
		return;
	}
	back.push(string(path));
	readcurr("../"); 
	return;
}

void press_enter()
{
	struct stat info;
	string content=contents[cursor+top-1];
	lstat(content.c_str(),&info);
	if(S_ISDIR(info.st_mode))
	{	
		if(strcmp(content.c_str(),"..")==0)
		{
			parent();
			return;
		}  
		if(strcmp(content.c_str(),".")==0) 
		{
			return;
		}
		back.push(string(path));
		readcurr((string(path)+'/'+string(content)).c_str());
	}
	else
	{
		pid_t pid=fork();
		if(pid==0)
		{
			cursor_pos(cursor,0);
			execl("/usr/bin/xdg-open","xdg-open",content.c_str(),NULL);
			exit(1);
		}
	}
	return;
}

void up()
{
	if(cursor>1)
	{
		cursor--;
		cursor_pos(cursor,0);
		return;
	}
	if(top==0)
	{	
		cursor_pos(cursor,0);
		return;
	} 
	top--;
	bottom--;
	showcurr();
	cursor_pos(cursor,0);
	return;
}

void down()
{
	total=contents.size();
	if(cursor<limit && cursor<total)
	{
		cursor++;
		cursor_pos(cursor,0);
		return;
	}
	if(bottom==total)
	{
		cursor_pos(cursor,0);
		return;
	} 
	top++;
	bottom++;
	showcurr();
	cursor_pos(cursor,0);
	return;
}

void slideUp()
{
    top=max(top-limit,0);
	bottom=top+limit;
	showcurr();
	cursor_pos(cursor,0);
	return;
}

void slideDown()
{
	bottom=min(bottom+limit,total);
	top=bottom-limit;
	showcurr();
	cursor_pos(cursor,0);
	return;
}

void go_back()
{
	if(back.size()==0)
	{
		return;
	}
	string prevdir=back.top();
	back.pop();
	forw.push(string(path));
	readcurr(prevdir.c_str());
	return;
}

void go_forw()
{
	if(forw.size()==0)
	{	
		return;
	}
	string nextdir=forw.top();
	forw.pop();
	back.push(string(path));
	readcurr(nextdir.c_str());
	return;
}

void go_root()
{
	if(path==rootpath)
	{
		return;
	} 
	back.push(string(path));
	readcurr(rootpath.c_str());
	return;
}


/*-----------Copy----------------*/

void copyfile_util(string fname,string p)
{
	char b[1000];
	int fin,fout,nread;
	fin = open(fname.c_str(),O_RDONLY);
	fout = open((p).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	while((nread = read(fin,b,sizeof(b)))>0)
	{
		write(fout,b,nread);
	}
}

void copyfolder_util(string dirName,string target)
{
	struct dirent *dir_obj;
	DIR *dir;
	struct stat info;
	if(!(dir=opendir(dirName.c_str())))
	{
		clrline;
		return;
	}
	chdir(dirName.c_str()); 
	while((dir_obj=readdir(dir)))
	{
		lstat(dir_obj->d_name,&info);
		string dname=string(dir_obj->d_name);
		if(S_ISDIR(info.st_mode))
		{	
			if((dname=="..")||(dname=="."))
			{
				continue;
			}	
			mkdir((target+'/'+dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
			copyfolder_util(dname,target+'/'+dname);
		}
		else
		{
			copyfile_util(dname,target+'/'+dname);
		}
	}
	chdir("..");
	closedir(dir);
	return;
}

void copyfile(int index)
{
	string target=commandArray[commandArray.size()-1];
	string fname=commandArray[index];
	string p=target+'/'+fname;
	copyfile_util(fname, p);	
}

void copyfolder(int index)
{
	string target=commandArray[commandArray.size()-1];
	string dname = commandArray[index];
	mkdir((target+'/'+dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
	copyfolder_util(path+'/'+dname,target+'/'+ dname);
}

/*-------------Delete---------------*/
void deletefile()
{
	string target=commandArray[1];
	unlink(target.c_str());
}
	
void deletefolder_util(string target)
{
	DIR *dir;
	struct dirent *dir_obj;
	struct stat info;
	if(!(dir=opendir(target.c_str())))
	{
		return;
	}
	chdir(target.c_str()); 
	while((dir_obj=readdir(dir)))
	{
		lstat(dir_obj->d_name,&info);
		if(S_ISDIR(info.st_mode))
		{
			if(strcmp("..",dir_obj->d_name)==0||strcmp(".",dir_obj->d_name)==0)
			{
				continue;
			}
			deletefolder_util(dir_obj->d_name);
			rmdir(dir_obj->d_name);
		}
		else
		{
			unlink(dir_obj->d_name);
		}
	}
	chdir("..");
	closedir(dir);
}

void deletefolder()
{
	string target=commandArray[1];
	if(target==path)
	{
		cout<<"Error";
		return;
	}
	deletefolder_util(target);
	rmdir(target.c_str());
}
/*------------Move--------------------*/

void movefile(int index)
{
	struct stat info;
	for(int i=1;i<commandArray.size()-1;i++)
	{
		string newpath=path+'/'+commandArray[i];
		lstat(newpath.c_str(),&info);
		if(S_ISDIR(info.st_mode))
		{
			copyfolder(i);
		}
		else
		{
			copyfile(i);
		}
	}
	string fname=commandArray[index];
	string p=string(path)+'/'+fname;
	unlink(p.c_str());
}

void movefolder(int index)
{
	string target=commandArray[commandArray.size()-1];
	string dname = commandArray[index];
	mkdir((target+'/'+dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
	copyfolder_util(path+'/'+dname,target+'/'+ dname);
	deletefolder_util(dname);
	rmdir(dname.c_str());
}

/*----Search--------*/

bool search_util(string dirName,string tobeSearch)
{

	struct dirent *dir_obj;
	DIR *dir;
	struct stat info;
	if(!(dir=opendir(dirName.c_str())))
	{
		return false;
	}
	chdir(dirName.c_str()); 
	while((dir_obj=readdir(dir)))
	{
		lstat(dir_obj->d_name,&info);
		string dname=string(dir_obj->d_name);
		if(tobeSearch==dname)
		{
			readcurr(dirName.c_str());
			return true;
		}
		if(S_ISDIR(info.st_mode))
		{
			if((dname=="..")||(dname=="."))
			{
				continue;
			}
			bool t=search_util(dirName+'/'+dname,tobeSearch);
			if(t)
			{
				return true;
			}
		}
	}
	chdir("..");
	closedir(dir);
	return false;
}
bool search()
{
	string str=commandArray[1];
	return search_util(path,str);	
}
/*--------------------------------------------------------------*/

void completetask(string task)
{
	if(task=="copy")
	{
		struct stat info;
		for(int i=1;i<commandArray.size()-1;i++)
		{
			string newpath=path+'/'+commandArray[i];
			lstat(newpath.c_str(),&info);
			if(S_ISDIR(info.st_mode))
			{
				copyfolder(i);
			}
			else
			{
				copyfile(i);
			}
		}
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Successfully copied."<<"\033[0m";
		cursor_pos(command_pos,17);
	}
	
	else if(task=="move")
	{
		struct stat info;
		for(int i=1;i<commandArray.size()-1;i++)
		{
			string newpath=path+'/'+commandArray[i];
			lstat(newpath.c_str(),&info);
			if(S_ISDIR(info.st_mode))
			{
				movefolder(i);
			}
			else
			{
				movefile(i);
			}
		}
		readcurr(path);
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Successfully copied."<<"\033[0m";
		cursor_pos(command_pos,17);
	}
	
	else if(task=="delete_file")
	{
		deletefile();
		readcurr(path);
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Successfully deleted."<<"\033[0m";
		cursor_pos(command_pos,17);
		
	}
	
	else if(task=="delete_dir")
	{
		deletefolder();
		readcurr(path);
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Successfully deleted."<<"\033[0m";
		cursor_pos(command_pos,17);
	}
	
	else if(task=="create_file")
	{
		string target=commandArray[commandArray.size()-1];
		for(int i=1;i<commandArray.size()-1;i++)
		{
			string fname=commandArray[i];
			open((target+'/'+fname).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
		}
		readcurr(path);
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Successfully created."<<"\033[0m";
		cursor_pos(command_pos,17);
	}

	else if(task=="create_dir")
	{
		string target=commandArray[commandArray.size()-1];
		for(int i=1;i<commandArray.size()-1;i++)
		{
			string fname=commandArray[i];
			mkdir((target+'/'+fname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
		}
		readcurr(path);
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Successfully created."<<"\033[0m";
		cursor_pos(command_pos,17);
	}

	else if(task=="rename")
	{
		string oldName=commandArray[1];
		string newName=commandArray[2];
		rename(oldName.c_str(),newName.c_str());
		readcurr(path);
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Successfully created."<<"\033[0m";
		cursor_pos(command_pos,17);
	}

	else if(task=="goto")
	{
		back.push(string(path));
		string target=commandArray[1];
		if(target=="/")
		{
			go_root();
		}
		else
		{
			readcurr(target.c_str());
		}
	}

	else if(task=="search")
	{
		bool f=search();
		if(f) 
		{
			cursor_pos(statusline,0);
			cout<<"\033[1;33m"<<"Found."<<"\033[0m";
			cursor_pos(command_pos,17);
		}
		else 
		{
			cursor_pos(statusline,0);
			cout<<"\033[1;33m"<<"Not Found."<<"\033[0m";
			cursor_pos(command_pos,17);
		}
	}	 

	else
	{
		cursor_pos(statusline,0);
		cout<<"\033[1;33m"<<"Wrong Command."<<"\033[0m";
		cursor_pos(command_pos,17);
	}

	return;
}


bool checkpath()
{
	int len=commandArray.size();
	string newpath=commandArray[len-1];

	if(newpath[0]=='/')			//relative path
	{
		newpath=path+newpath;
	}
	
	else if(newpath[0]=='~')		//absolute path from root
	{
		newpath.replace(0,1,rootpath);
	}
	
	else if(newpath[0]=='.')
	{
		newpath=path;
	}
	
	else 
	{
		return false;
	}
	
	commandArray[len-1]=newpath;
	return true;
}


string typing()
{
	string s;
	cursorIndex=17;
	cursor_pos(command_pos,17);
	while(1)
	{
		char ch=cin.get();
		
		if(ch==10)
		{
			cursorIndex=17;
			cursor_pos(command_pos,cursorIndex);
			return s;
		}
		if(ch==27)
		{
			return "Esc";
		}
		else
		{
			if(ch==127)
			{
				if(cursorIndex==17)
				{
					continue;
				}
				
				s.pop_back();
				cursorIndex--;
				cursor_pos(command_pos,0);
				cout<<"\033[1;32m"<<"Command-Mode: $ "<<"\033[0m";
				cursor_pos(command_pos,17);
				cout<<"\033[K";
				cout<<s;
				cursor_pos(command_pos,s.size()+17);
			}
			else
			{
				s.push_back(ch);
				cout<<ch;
				cursorIndex++;
				cursor_pos(command_pos,cursorIndex);
			}
		}
	}
	return "1";
}


void enableCommandMode()
{
	//clr;
	while(1)
	{
		cursor_pos(command_pos,0);
		cout<<"\033[1;32m"<<"Command-Mode: $ "<<"\033[0m";
		cursor_pos(command_pos,17);
		commandArray.clear();
		string s=typing();
		string part,task;
		if(s=="Esc")
		{
			cursor_pos(cursor,0);
			showcurr();
			break;
		}
		else
		{
			clrline;
		}
		stringstream ss(s);
		while(getline(ss,part,' '))
		{
			commandArray.push_back(part);
		}
		task=commandArray[0];
		int valid=checkpath();
		if(!valid && task!="search" && task!="rename")
		{
			clrline;
			cursor_pos(command_pos,0);
			cout<<"\033[1;32m"<<"Command-Mode: $ "<<"\033[0m";
			cursor_pos(command_pos,17);
			continue;
		}
		completetask(task);
	}
	return;
}


int main()
{
	clr;
	cout<<"\e[8;25;140t";		//adjust the terminal window size
	enableRawMode();
	getcwd(path,1000);		//get current working directory
	initialize(path);
	cursor_pos(cursor,0);
	readcurr(path);
	char ch;
	while(ch!='q')
	{
		ch=cin.get();
		switch(ch)
		{
			case 10:	press_enter();
					break;
			
			case 65: 	up();			//up arrow key
					break;
					
			case 66: 	down();			//down arrow key
					break;
					
			case 67: 	go_forw();		//right arrow key
					break;
					
			case 68: 	go_back();		//left arrow key
					break;							  
			
			case 107:	slideUp();		//press k
					break;
					
			case 108:	slideDown();	//press l
					break;
							
			case 104:	go_root();		//press h
					break;
							
			case 127:	parent();		//backspace
					break;
					
			case ':':	enableCommandMode();
					break;
					
			default: 	break;
		}
	}
	clr;
	atexit(disableRawMode);
	return 0;
}
