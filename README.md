kinectTrackerRealTime
=====================
In this release, two-Kinect tracker module is built as a client-server architecture. Each Kinect is connected to a computer, that extracts the point-clouds of the users in the scene, removing the background, floor and static objects, and sends it to the central server via the OSC network module. The server merges the data coming from the other processes and computes the significant data that feeds the control system.
