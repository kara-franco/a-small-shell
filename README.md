# a-small-shell
A shell written in C that runs basic command line instructions from Operating Systems Course  

The shell supports three built in commands: exit, cd, and status. It also supports comments, which are lines beginning with the # 
character. The general syntax of a command line is: command [arg1 arg2 ...] [&lt; input_file] [&gt; output_file] [&amp;]. Items in 
square brackets are optional. The special symbols &lt;, &gt;, and &amp; are recognized, but they must be surrounded by spaces like 
other words. If the command is to be executed in the background, the last word must be &amp;. If standard input or output is to be 
redirected, the &gt; or &lt; words followed by a filename word must appear after all the arguments. Input redirection can appear 
before or after output redirection.

Here is an example of the shell in use:

$ gcc -o smallsh smallsh.c

$ smallsh
: ls
junk smallsh smallsh.c
: ls &gt; junk
: status
exit value 0
: cat junk
junk
smallsh
smallsh.c
: wc &lt; junk
3 3 23
: test -f badfile
: status
exit value 1
: wc &lt; badfile
smallsh: cannot open badfile for input
: status
exit value 1
: badfile
badfile: no such file or directory
: sleep 5
^Cterminated by signal 2
: status
terminated by signal 2
: sleep 15 &amp;
background pid is 4923
: ps
PID TTY TIME CMD
4923 pts/4 0:00 sleep
4564 pts/4 0:03 tcsh-6.0
4867 pts/4 1:32 smallsh
:
:
: # that was a blank command line, this is a comment line
background pid 4923 is done: exit value 0
: # the background sleep finally finished
: sleep 30 &amp;
background pid is 4941
: kill -15 4941
background pid 4941 is done: terminated by signal 15
: pwd
/student/francok/CS344/prog3
: cd
: pwd
/student/francok
: cd CS344
: pwd
/student/francok/CS344
: exit
$
