#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

//JOB STRUCT
struct job
{
	int id;
	pid_t pid;
	char command[100];
	int status;
};

#define JOB_RUNNING 1
#define JOB_STOPPED 2
#define JOB_DONE 3
struct job jobs[10];
int job_count = 0;

// add a job
void add_job(pid_t pid, char *command)
{
	if(job_count >= 10)
	{
		printf("job table full \n");
		return;
	}

	jobs[job_count].id = job_count +1;
	jobs[job_count].pid = pid;

	strncpy(jobs[job_count].command,
		command,
		sizeof(jobs[job_count].command)-1);

	
	jobs[job_count].command[
		sizeof(jobs[job_count].command) -1] = '\0';

	jobs[job_count].status = JOB_RUNNING;

	job_count++;
}
//FIND A JOB USING PID

int find_job(pid_t pid)
{
	for(int i = 0; i<job_count; i++)
	{
		if(jobs[i].pid == pid)
		{
			return i;
		}
	}
	return -1;
}

// FIND JOB USING JOB ID

int find_job_by_id(int job_id)
{
    for (int i = 0; i < job_count; i++)
    {
        if (jobs[i].id == job_id)
        {
            return i;
        }
    }

    return -1;
}

//SIGCHLD HANDLER
void handle_sigchld(int sig)
{
	int status;
	pid_t pid;


	/*collect all finished child process.
	
	WNOHAND MEANS:
	if no child has finihses, dont wait*/
	while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
{
    int index = find_job(pid);

    if (index != -1)
    {
        if (WIFSTOPPED(status))
        {
            jobs[index].status = JOB_STOPPED;
        }
        else if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            jobs[index].status = JOB_DONE;
        }
    }
}
}
	//child process celaned up

// PRINT JOBS
void print_jobs()
{
    for (int i = 0; i < job_count; i++)
    {
        char *state;

        if (jobs[i].status == JOB_RUNNING)
        {
            state = "Running";
        }
        else if (jobs[i].status == JOB_STOPPED)
        {
            state = "Stopped";
        }
        else
        {
            state = "Done";
        }

        printf("[%d] %d %s    %s\n",
               jobs[i].id,
               jobs[i].pid,
               state,
               jobs[i].command);
    }
}

// FOREGROUND JOBS

void foreground_job(int job_id)
{
    int index = find_job_by_id(job_id);

    if (index == -1)
    {
        printf("fg: no such job\n");
        return;
    }

    if (jobs[index].status == JOB_DONE)
{
    printf("fg: job %d is already done\n", job_id);
    return;
}

    printf("Bringing job [%d] to foreground: %s\n",
           jobs[index].id,
           jobs[index].command);

    /*
    Wait for this particular background process.
    */

    waitpid(jobs[index].pid, NULL, 0);

    jobs[index].status = JOB_DONE;
}

// BACKGROUND JOBS

void background_job(int job_id)
{
    int index = find_job_by_id(job_id);

    if (index == -1)
    {
        printf("bg: no such job\n");
        return;
    }

    if (jobs[index].status == JOB_DONE)
{
    printf("bg: job %d is already done\n", job_id);
    return;
}

    /*
    SIGCONT tells Linux to continue
    a stopped process.
    */

    if (kill(jobs[index].pid, SIGCONT) == -1)
    {
        perror("bg");
        return;
    }

	jobs[index].status = JOB_RUNNING;

    printf("[%d] %s &\n",
           jobs[index].id,
           jobs[index].command);
}



// MAIN FUNCTION
int main()
{
	char input[100];
	char *args[10];

	// INSTALL SIGCHILD HANDELR

	signal(SIGCHLD, handle_sigchld);
	// ignore the sigint command for main shell running
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	while(1)
{
		int i = 0;

		printf("Mini Shell> ");

		fgets(input, sizeof(input),stdin);

// remove new line from input
	input[strcspn(input, "\n")] = '\0'; //strcspn find the index of \n in input

//split input into  words
	char *token = strtok(input, " ");

	while (token != NULL)
{
		args[i] = token;
		i++;

		token = strtok(NULL, " ");
}
	args[i] = NULL;

//built in exit: exit

	if (strcmp(args[0], "exit")==0)
{
	break;
}

// built in: cd
	if (strcmp(args[0], "cd")==0)
{
		if (args[1] == NULL)
{
			printf("cd: missing argument \n");
}
		else
{
			if(chdir(args[1]) !=0)
{
				perror("cd");
}
}
		continue;
}

//NEW JOBS BBUILT IN

	if (strcmp(args[0], "jobs") == 0)
	{
		print_jobs();
		continue;
	}

// FG BUILT IN
if (strcmp(args[0], "fg") == 0)
        {
            if (args[1] == NULL)
            {
                printf("fg: missing job ID\n");
            }
            else
            {
                int job_id = atoi(args[1]);

                foreground_job(job_id);
            }

            continue;
        }

// BG BUILT IN

if (strcmp(args[0], "bg") == 0)
        {
            if (args[1] == NULL)
            {
                printf("bg: missing job ID\n");
            }
            else
            {
                int job_id = atoi(args[1]);

                background_job(job_id);
            }

            continue;
        }

// NEW check for background &

	int background = 0;

	for(int j = 0; args[j]!= NULL; j++)
{
	if (strcmp(args[j], "&") == 0)
	{
		background = 1;

		// REMOVE & FORM ARGUMENTS
		args[j] = NULL;

		break;

	}
}
//NEW PIPE MULITIPLE PIPES

	int pipe_count = 0;
	for(int j = 0; args[j] != NULL; j++)
{
		if(strcmp(args[j], "|") == 0)
{
			pipe_count++;
}
}

//HANDEL PIPES
	if(pipe_count >0)
{
		char **commands[10];

		int command_count = 0;

		commands[command_count]= args;

		command_count++;

		for(int j = 0; args[j] != NULL;j++)
{
			if (strcmp(args[j], "|") == 0)
{
				args[j] = NULL;

				commands[command_count] = &args[j+1];

				command_count++;
}
}
	//create all required pipes

	int pipes[9][2];

	for(int j = 0; j < pipe_count; j++)
{
		if(pipe(pipes[j]) == -1)
{
			perror("pipe");
			continue;
}
}
	pid_t pids[10];

	for(int j = 0; j < command_count; j++)
{
		pids[j] = fork();

		if (pids[j] == 0)
{

//connect input from previous pipe
			if(j>0)
{
				dup2(pipes[j-1][0], STDIN_FILENO);
}
//CONNECT OUTPUT TO NEXT PIPE

			if(j<command_count -1)
{
				dup2(pipes[j][1], STDOUT_FILENO);
}

//CLOSE ALL TYPES OF FILE DESCRIPTORS
	for(int k = 0; k< pipe_count; k++)
{
		close(pipes[k][0]);
		close(pipes[k][1]);
}
//EXECUTE COMMAND
		execvp(commands[j][0], commands[j]);

		perror("exec");
		return  1;
}
}
	//parent closes pipe
	for(int j = 0; j<pipe_count; j++)
{
	close(pipes[j][0]);
	close(pipes[j][1]);
}
	//wait for both all children
 	for (int j = 0; j<command_count; j++){
	waitpid(pids[j], NULL, 0);
}

	continue;
}

// check for output redirection(>)

	int redirect = 0;
	int append_output = 0;
	char *output_file = NULL;

	for (int j = 0; args[j] != NULL; j++){

//CHECK FOR >>
		if (strcmp(args[j], ">>") == 0){

			append_output = 1;

			if(args[j+1] != NULL)
{
				output_file = args[j+1];
}
			args[j] = NULL;

		break;
}

//EXISTING CHECK FOR >
	if(strcmp(args[j], ">") ==0){

		redirect = 1;

		if (args[j+1] != NULL){

			output_file =  args[j+1];
}
		args[j] = NULL;
		break;
}
}
// check for input redirection

	int redirect_input = 0;
	char *input_file = NULL;

	for (int j = 0; args[j] != NULL; j++)
{

	if(strcmp(args[j], "<") ==0){

	redirect_input = 1;

		if(args[j+1] != NULL){
			input_file = args[j+1];
}

	args[j] = NULL;

	break;
}
}

// create child process
	pid_t pid = fork();

	if (pid == 0){

		setpgid(0, 0);

		signal(SIGINT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);

		if(redirect ==1){
			int fd = open(output_file,
			O_WRONLY | O_CREAT | O_TRUNC,
			0644);

// output redirection
			if (fd<0){
				perror("open");
				return 1;
}
			dup2(fd, STDOUT_FILENO);
			close(fd);
}

//APPEND OUTPUT REDIRECTION

	if(append_output == 1)
{
		int fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);

		if(fd<0)
{
		perror("open");
		return 1;
}
	dup2(fd, STDOUT_FILENO);
	close(fd);
}
//input redirection

	if(redirect_input ==1){

		int fd = open(input_file, O_RDONLY);
		if (fd<0){

			perror("open");
			return 1;
}

		dup2(fd, STDIN_FILENO);
		close(fd);
}

//execute command
	execvp(args[0], args);

	perror("exec");
	return 1;
}
	if (background == 1)
{
	printf("[started] %d \n", pid);
	add_job(pid, input);
	//DONT WAIT
}
	else
{
	int status;
		// FOREGROUND PROECSS
		waitpid(pid, &status, WUNTRACED);

}
}
	return 0;
}
