# Cloudflare Internship Application: Systems

## What is it?

Please write a small Ping CLI application for MacOS or Linux.
The CLI app should accept a hostname or an IP address as its argument, then send ICMP "echo requests" in a loop to the target while receiving "echo reply" messages.
It should report loss and RTT times for each sent message.


## Compiling & Running the code
__Compile__
```
$ g++ main.cpp -o main
```

__Run__  
Make sure that you run the executable in sudo mode.
```
$ sudo ./main [-c count] [-w wait] [-t ttl] <hostname>
```

## Flags
```
[-w wait] 
Wait seconds between sending each packet. The default is to wait for one second between each packet.

[-c count]
Stop after sending (and receiving) count ECHO_RESPONSE packets.

[-t ttl]
Time to live or hop count of the packets.
```