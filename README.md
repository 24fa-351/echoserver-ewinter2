# 351-redirect
empty for project

Using TCP, create a server that echoes all messages it receives, line by line.

If you send "abcd"(newline), you should get back "abcd"(newline). Then type another line, the same thing. As long as the connection stays alive. 

Implement a "-p" argument that specifies the port it listens to.

It only needs to accept one connection at a time, but it must accept new connections after one is terminated.

Test this with telnet; you'll probably need to install it.

Highly recommended: test it with your web browser, too.

EC: 25 more points for making it multi-threaded. (really easy!)

 