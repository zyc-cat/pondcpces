DESIGN
======

This client communicates between the IPPC-2011 server and POND through two different socket connections.

The user gives the client a set of problems to solve.  The client then prioritizes the list of problems,
creates a new process for POND, and solves a problem with POND in an online fashion.  The POND process
is terminated upon completion of the problem, and a new process is created for the next problem.  If
POND crashes prematurely, then the client will restart POND and lead it to the state in which it left off.
The client may potentially re-prioritize the list of problems after completion of each problem.  The
client may also terminate POND early because of time or other reasons.

Functions
---------

- Connect to Server
- Get Next Best Problem
- Connect to POND with Problem
- [[ Interact with Server ]]
  - Get Observations
  - Send Action(s) (Translated)
- [[ Interact with POND ]]
  - Send Observations (Translated)
  - Get Actions
  - Send Action History (As Recovery from Crash)
  - Terminate

Command-line
------------

Run "./rddlclient -h" for help
