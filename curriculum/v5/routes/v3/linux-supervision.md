# Linux service supervision

Start with a single worker. Give each observation an observer and a time. One
process can have a PID in its own namespace and a different PID as seen by its
parent outside that namespace. Record the relationship while both processes are
alive. Later, use the same discipline for a socket address. Include the network
namespace that contains the socket when explaining a bind result.

A process identifier gives you a way to refer to the worker. To explain why it
could not start another worker, compare the attempted operation with its cgroup
limits and events. For shutdown, keep a record per owned child from launch
through collection of its exit status. A signal request records one step in
that sequence. Only a terminal wait result records the collected child.

Follow the exercise contract for each observation. Distinguish a supplied
trace from a run you performed, and include the exact reason whenever the host
denies an operation. The fifth exercise is an elective diagnosis available
after the four required investigations.
