#include "Shell.h"
/* Constructor */
Shell::Shell(){
  parallels = 0;
  TMP_PIPE = "/tmp/fifo_one";
  TMP_FILE = "/tmp/file_one";
  tmp_file_str = "/tmp/file_one";
  tmp_file_full = false;
  prompt = "user@shell:>>";
}

/* All functionality happens inside shellMain */
void Shell::shellMain(){
  // Initialize
  string line, command, args, pipe_arg = "", tmp_dir,
		  file_name, quote_string, dir_path, last_dir_path;
  ofstream tmpfile, writefile;
  bool get_args, loop = true;
  char ch;
  mkfifo(TMP_PIPE, 0644);
  stringstream ss;

  /* While loop goes until 'exit' is typed */
  while(loop){
    // Clear buffers
	tmp_file_full = false;
    ss.clear();
    line = "";

    cout << prompt;
    getline(cin, line);
    ss << line;
    while(getline(ss >> ws, command, ' ')){
      args = command + " "; // Command must be first arg
      get_args = true;

      /****************** Special Cases *****************/
      if(command == "exit"){ // Exit case for EZ escape
    	  cout << "goodbye." << endl;
    	  if(parallels){ // No zombie processes pls and thank u
    		  wait(NULL);
    		  --parallels;
    	  }
    	  return;
      }
      if(command == "cd"){ // Change directory acts like a special snowflake
    	  getline(ss >> ws, dir_path, ' ');
    	  if(dir_path == "-"){
    		  last_dir_path.erase(last_dir_path.length()-1);
    		  tmp_dir = pipeExec("pwd", "");
    		  chdir(last_dir_path.c_str());
        	  last_dir_path = tmp_dir;
    	  }
    	  else {
        	  last_dir_path = pipeExec("pwd", "");
    		  if(chdir(dir_path.c_str()) == -1){
    			  cout << "invalid path " << dir_path << endl;
    		  }
    	  }
    	  get_args = false;
      }

      /************ TOKENIZER ************/
      while(ss.get(ch) && get_args){ // token = single character
    	file_name = "";
        switch(ch){
          case '|': // CASE : Pipe to another process
        	if(tmp_file_full){
        		args += " " + tmp_file_str;
        		tmp_file_full = false;
        	}
            pipe_arg = pipeExec(command, args);
            tmpfile.open(tmp_file_str);
            tmpfile << pipe_arg;
            tmpfile.close();
            get_args = false;
            tmp_file_full = true;
            break;
          case '<': // CASE : Add file to arg list (pipe in)
        	ss.get(ch);
        	getline(ss >> skipws, file_name, ' ');
        	if(!(ch == ' ')){
        		file_name = ch + file_name;
        	}
        	args += " " + file_name;
        	break;
          case '>': // CASE : pipe to a file (pipe out)
          	ss.get(ch);
          	getline(ss >> skipws, file_name, ' ');
          	if(!(ch == ' ')){
          		file_name = ch + file_name;
          	}
        	writefile.open(file_name);
        	if(tmp_file_full){
        		args += " " + tmp_file_str;
        		tmp_file_full = false;
        	}
        	pipe_arg = pipeExec(command, args);
        	writefile << pipe_arg;
        	writefile.close();
        	get_args = false;
        	break;
          case '\"': // CASE : Handle spaces inside quotes
        	getline(ss, quote_string, '\"');
        	args = args + " " + "\"" + quote_string + "\"";
        	break;
          case '\'': // CASE : Handle spaces inside single quotes
        	getline(ss, quote_string, '\'');
        	args = args + " " + "\'" + quote_string + "\'";
        	break;
          case '&': // CASE : background process
        	if(tmp_file_full){
        		args += " " + tmp_file_str;
        	}
        	parallelExec(command, args);
            get_args = false;
            break;
          default:
            args += ch;
            break;
        }
      }

    }
    if(tmp_file_full){
  	  args += " " + tmp_file_str;
  	  tmp_file_full = false;
    }
    if(get_args){ // Run the final command
    	shellExec(command, args);
    }
    cout << endl;
  }
  while(parallels){ // Before returning make sure that there are no parallels running
  	wait(NULL);
  	--parallels;
  }
  return;
}

/*
 * Different shellExec helpers:
 * ---shellExec is the default execution
 * ---parallelExec will be the execution call for '&' symbol
 * ---pipeExec returns the standard output of the process as a string for
 * use in the '|' case
 */
void Shell::shellExec(string com, string arg){
  int pid = fork();
  if(!pid){ // Child
	char** args = strToCharArr(arg);
	if(args[0] == NULL){
		args[0] = "";
	}
    execvp(com.c_str(), args);
    cout << "Process " << com << " failed to start from shellExec.\n";
    exit(1);
  }
  else{
	// DEBUG
    wait(NULL);
    return;
  }
}

void Shell::parallelExec(string com, string arg){
  int pid = fork();    while(parallels){
  	wait(NULL);
  	--parallels;
  }
  if(!pid){
	  char** args = strToCharArr(arg);
	  if(args[0] == NULL){
		  args[0] = "";
	  }
	  execvp(com.c_str(), args);
	  cout << "Process " << com << " failed to start from shellExec.\n";
	  exit(1);
  }
  else{ // No wait for child, but alerts Shell that it needs to wait later
	  ++parallels;
	  return;
  }
}

string Shell::pipeExec(string com, string arg){
    int pipefd, pid;
    pid = fork();
    if(!pid){ // Child
	pipefd = open(TMP_PIPE, O_WRONLY);
    dup2(pipefd, STDOUT_FILENO);
    close(pipefd);
    char** args = strToCharArr(arg);
	if(args[0] == NULL){
		args[0] = "";
	}
    execvp(com.c_str(), args);
    cout << "Process " << com << " failed to start from pipeExec.\n";
    exit(1);
  }
  else{ // Parent
    string output;
    pipefd = open(TMP_PIPE, O_RDONLY);
    char buf[512];
    while(read(pipefd, buf, 512)){
      output += buf;
    }
    wait(NULL);
    close(pipefd);
    return output;
  }
}

/* string to char is hard :( */
char** Shell::strToCharArr(string s){
	for(int i = s.length()-1; i > 0; --i){
		if(s[i] == ' '){
			s.erase(i, 1);
		}
		else {
			break;
		}
	}
	vector<string> args;
	string tmp;
	int maxi = 0;
	char ch;
	bool dq_ignore = false, sq_ignore = false; // dq = double quote, sq = single quote
	for(int i = 0; i < s.length(); ++i){
		ch = s.at(i);
		if(ch == '\"'){
			if(dq_ignore){
				dq_ignore = false;
			}
			else{
				dq_ignore = true;
			}
		}
		else if(ch == '\''){
			if(sq_ignore){
				sq_ignore = false;
			}
			else{
				sq_ignore = true;
			}
		}
		else if(ch == ' ' && !(sq_ignore || dq_ignore)){
			if(tmp.length() > maxi){
				maxi = tmp.length();
			}
			if(tmp != "" && tmp != " "){
				args.push_back(tmp);
			}
			tmp = "";
		}
		else{
			tmp += ch;
		}
	}
	args.push_back(tmp);
	int arg_count = 0;
	char* buf;
	char** ret = NULL;
	for(int i = 0; i < args.size(); ++i){
	  ret = (char**)realloc((void*)ret, sizeof(char*) * ++arg_count);

	  if (ret == NULL)
	    exit (1);
	  buf = new char[maxi];
	  strcpy(buf, args[i].c_str());
	  ret[arg_count-1] = buf;
	}
	ret = (char**)realloc((void*)ret, sizeof(char*) * (arg_count+1));
	ret[arg_count] = NULL;
	// DEBUG
	/*
	for (int i = 0; i < (args.size()+1); ++i)
	  printf ("newret[%d] = %s\n", i, ret[i]);
	*/
	return ret;
}
