# Panda

Panda is an experiment in **P**arametric **A**nimation a**n**d **D**rawing **A**rchitecture.

It proposes a visual programming languages to create and edit images and react to user input in real time.

## Concept

The user creates a graph of objects (boxes on the screen) that process data. Each object does a simple task (addition, multiplication, access to an item in a list, etc.), and by combining them we visually create a program. Most of the objects have inputs (on the left) and outputs (on the right). A program is then usually read left to right.   
Around 300 objects have been created so far, that generate data, modify it, do computation on it or transform it, save it or render something on screen based on the inputs.

![A typical program](http://i.imgsafe.org/81ca352.jpg)

Most of the programs are created to render something on the screen, which is an OpenGL view on the *Render* tab. Some do animations by changing values at each timestep (it is necessary to *Play* them - F5 or the button on the toolbar). Others react to the user input, mouse movement and left clicks.

![Render view](http://i.imgsafe.org/86a6113.jpg)

### Types

Multiple types are supported, but in general each object does process only one type for each input. Here is the list of all types:

- Integer
- Real
- Point (2d)
- Rectangle
- Color
- String
- Image (either read from a file, a texture, or a framebuffer object)
- Gradient of colors
- Path (continuous list of points)
- Polygon (a contour and optionally some holes)
- Mesh (lists of points, edges and triangles)
- Shader (multiple programs and their uniform inputs)
- Lists of all the above
- Animations of the types that can be interpolated (Real, Point, Color, Gradient). An animation has keys and values, an interpolation mode, and a method for computing out of bounds values.

#### Conversion

Some types can be implicitly converted to others, for example Real to Integer and back. All single values are convertible to a list containing one value. Converting a list to a single value extracts the first value if it exists (otherwise, it gets the default value for this type). Any animation can be converted to the corresponding list of values, but not the other way around.

## Creation of a program

In order to create a program, the user adds objects to the document, set their parameters, and link them to one another.

### Adding an object 

The *Add* menu show all objects that can be created. 

![Add Menu](http://i.imgsafe.org/7ba4e45.png)

There is also a dialog box invoked by pressing space while the graph view is in focus that will allow to search objects by name. When multiple objects are shown but none are selected, pressing enter will add the first in the list.

![Quick Add dialog](http://i.imgsafe.org/7c89c9d.png)

### Properties edition

The parameters of the selected object are shown in the *Properties* dock, by default on the left of the GUI.  
A button with the text "..." means that a dialog box can be opened by clicking on it for further edition (for colors, gradients and all lists).  
Output or connected input values are ready only.

![The properties dock](http://i.imgsafe.org/724fffe.png)

### Variety of objects

All objects have inputs and or outputs. These are represented by small squares in each object, on the left for inputs and right for outputs. 

![Basic types of objects](http://i.imgsafe.org/70d1e47.png)

Hovering over a property will show a box with its description and its type.

![Tooltip for properties](http://i.imgsafe.org/d6210a6.png)

Most of the object have a rectangle shape, with rounded corners. These transform data in one form or another, but have no side effects.

#### Groups

The objects having beveled corners are called **groups**. These contain other objects inside of them (see further down for more information). They usually also process data.

![A group](http://i.imgsafe.org/8e0a5f3.png)

#### Docks

Some objects can modify the behavior of others. For example, the *render Disk* object can either render a disk on the screen, or on a texture. In the image below, it will do the rendering in the layer it is connected to.  
One other application of the dock concept is the *Particle system*, where other objects add behavior to it (creation, destruction, forces).

![Docks](http://i.imgsafe.org/70072ee.png)

#### Generic input

Some objects have inputs that can accept many types of values. 

![Generic input](http://i.imgsafe.org/6f61418.png)

When connecting to this input, the object will create actual inputs and outputs for the specific type connected, growing vertically. These objects can do the same operation on many types, and are used in programs to represent a transformation occurring at once on multiple data. 

![Generic object](http://i.imgsafe.org/72a7a61.png)

### Connecting objects

Objects are linked by their properties.  
Hovering over any property will highlight all links and properties connected to it.

![A link](http://i.imgsafe.org/717dab4.png)

A link is created by dragging from a property to another compatible one: inputs are connected to outputs, the type of the values must be the same or implicitly convertible. While dragging, incompatible properties are grayed out.

![Connecting](http://i.imgsafe.org/ef2fd22.png)

Links are only drawn when the input is on the left of the output. Otherwise, the line is replaced by two little boxes with the same number.

![Also a link](http://i.imgsafe.org/73037c9.png)

A search is done when connecting to ensure no circular graph is created, which would run the program in an infinite loop.  
Currently three objects allow some form of loop in a program:
- **Buffer**: when the control input is modified, it copied all inputs to outputs. Otherwise all modifications to the inputs are not propagated.
- **Loop**: when the control input is modified, it starts a *while* loop. It update all inputs until the condition input is false or the set number of iterations is reached.
- **Replicator**: This is a Layer that run the list of render objects connected to it multiple times.

## Other features

Undo - Redo stack  
Copy & Paste, even across applications (it is XML)  
Save images and values to files  
Annotations to visually comment the program  
Alignment, snap to neighbors and distribution of the objects in the graph view  
Open or detach new tabs to view the content of any image in a program  
Multi-thread run of the program using a scheduler. 

### Layers

Render objects can be organized in Layers. Then as in every photo editing software, the order of the layers can be changed, their opacity and blend mode modified.

![Layers](http://i.imgsafe.org/066646a.png)

### Time log

Even if Panda is not extremely optimized (yet ?), it still wants to be as fast as possible in order to get 60 FPS on complex programs. A log is created for each timestep, which can be accessed in *View/show log*.  
In the dialog box we can see a detailed graph of the events that occurred (updates and access to properties). While the log is shown, the graph view is modified to show the status of objects and properties at the time represented by the red line in the log (red is dirty, green is updated).

![Time log](http://i.imgsafe.org/88cf025.jpg)

### Groups

When creating a useful program, it is not practical to copy-paste it every time you want to reuse it. Like a function in any programming language, here it is possible to group a selection. This converts any number of objects into a single one that will hide the selection. The created group can be saved to file and will appear in the *Add* menu like any standard object. Its inputs and outputs can be renamed, and a description added to document its use.

Below is shown the *Mouse trace* group, its contents when expanded, and the dialog box used to rename its inputs and outputs.

![Editing a group](http://i.imgsafe.org/8340b69.jpg)

