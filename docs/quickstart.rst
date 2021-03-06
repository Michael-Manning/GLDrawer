Quick Start
============

To install GLDrawer in your project follow these steps.

| **Step 1**
| Download The .DLL from the Github repository `here.`_

.. _here.: https://github.com/Michael-Manning/GLDrawer

| **Step 2**
| Extract the files, locate the latest release of GLDrawer, then copy the GLDrawer.DLL file.
| Place the .DLL file somewhere easy to find or in your project directory.

| **Step 3**
| Open your project. In the solution explorer, right click references, then click add reference.
| Click Browse, navigate to GLDrawer.DLL and hit okay.

| **Step 4**
| Include the GLDrawer namespece in your file with the "using" keyword and test the following program.

.. code-block:: C#

    using GLDrawer;
 
    namespace myProject
    { 
        public static class Program
        {
            static void Main(string[] args)
            {
                GLCanvas can = new GLCanvas();
                can.AddText("It Works :)");
                Console.ReadKey(); 
            }
        }
    }

A window should will pop up and you should see the "it works" text. 
If you did, GLDrawer has been installed successfully!

**Issues?**

For GLDrawer to work, your project must be target .NET Framework 4.5 or earlier.
You also may need to tell visual studio to Prefer 32-bit or even target x86 for your CPU.

Right click your project in the solution explorer and click properties. You will find these two settings
under Application and Build respectively.

.. image:: images/targetCPU.png
   :width: 670px
   :height: 377px
   :scale: 100 %
   :alt: Choose x86 target CPU
   :align: left

