# SysNets2Project2
Repo for SN2 Project 2, I'll update this description after due date to be a little more descriptive

Message formats:

Token:
<token>
//NOPE sharefilename.txt //the name of the shared bb file
numberOfClients
ip port //(first client in the ring, this will be shifted every time the token is transfered from one client to the next)
ip port //2nd client
...
ip port //last client)
message	//optional message for the client
</token>

Join request:
<join request>
ip port //this is the client's ip address and port number they can be reached with
filename //this is the filename of the bbfile we want to write and read to. 
</join request>