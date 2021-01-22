# Node-Based Fabric Simulation in C
A simple, efficient program for simulating a node-based fabric, mainly for aesthetic purposes.
The program stores heights of a grid of nodes and applies force on these heights if the distance to an adjacent node exceeds the maximum distance, the height exceeds the maximum height, or a force is applied by the cursor.
In addition, the fabric will gradually fall back into a 2-D sine wave when no external force is applied.
All of these features and many others can be customized by changing the constants at the top of main.c.

There are also several draw modes that can render the fabric in different ways using Raylib.
It can draw a circle or square for each node, draw a grid where each vertex is a node, or draw diagonal lines connecting the nodes.
These can also be customized and combined using the constants in main.c.
I recommend not using the circle draw mode as it causes a significant decrease in performance.
The following is a demonstration of the square drawing mode with square size equal to the spacing between nodes on the screen.

![](demo.gif)

A vast amount of customization is possible in many different ways with the parameters of the simulation.
Recordings for the simulations must have high resolution and framerate, making it difficult to upload demos for this, but I highly recommend downloading the program and exploring the effects that the different parameters have.