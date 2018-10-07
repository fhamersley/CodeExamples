## Code from MTRN3500 Assignment 2 - Computing Applications in Mechatronic Systems

The assignment involved developing a multi module system for the purpose of remote tele-operation of a UGV. Shared memory was used to communicate between modules with the process management module recognising when modules were not correctly responding. Appropriate action needed to take place when modules failed to respond depending on if they were critical modules or not.

Data was received from several hardware components on the UGV such as a SICK LMS151 laser rangefinder, a Novatel SMART-VI GPS unit and USB webcam. Data was transmitted over TCP and was displayed in a simulated OpenGL environment. The UGV was also controlled locally using an Xbox controller with the controls also being transmitted back to the UGV using TCP.
