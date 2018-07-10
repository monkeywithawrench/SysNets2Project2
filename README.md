# SysNets2Project2
Repo for SN2 Project 2.
This project creates a token ring and a bulletin board where clients can post messages. The server program establishes the token ring, while instances of the client programs maintain the token ring and ensure only 1 client has an access token at any given time.
Each client instance forks into 2 separate threads, each making sure race conditions are not possible. 1 thread manages the token ring client, the other thread manages user input, as well as reading and writing to the bulletin board. The bulletin board thread can only write to the bulletin board when the client has the access token. See User Manual for more info.

Message formats:

Token:
<token>
numberOfClients
ip port //(first client in the ring, this will be shifted every time the token is transfered from one client to the next)
ip port //2nd client
...
ip port //last client)
message	//optional message for the client
</token>

Join request:
<join request>
filename //this is the filename of the bbfile we want to write and read to. 
ip port //this is the client's ip address and port number they can be reached with
</join request>

