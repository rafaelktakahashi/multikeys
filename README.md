# Introduction 
Custom keyboard layouts are an option to those who want to expand the functionality of their keyboards, whether to type uncommon symbols or to define custom macros.  
Many applications for creating custom layouts exist, such as [AutoHotKey][AutoHotKey] and [Microsoft's Keyboard Layout Creator][MSKLC], with varying capabilities and learning curves. However, they are not capable of remapping more than one keyboard simultaneously.  
The issue is that the Windows operating system does not allow multiple keyboard layouts to be used at once. If more than one keyboard is connected to the computer, they all behave as if they were the same device (for example, they share the capslock state), which complicates the task of remapping keys from specific keyboards.  
Typically, keystrokes are intercepted using either a Keyboard Hook or the RawInput API, both from the Windows API. However, [this article][Blecha on Remapping] by Vít Blecha describes how to selectively block keyboard keystrokes based on which device the signal came from, showing that it should be possible for an application to treat keystrokes differently for different keyboards. That, combined with an internal model of a keyboard layout, should enable a user to effectively remap multiple keyboards using different layouts.  
This application aims at providing such a solution: to allow the user to create and use multiple keyboard layouts at once.

# Getting Started
This app is developed in Visual Studio 2017 targeting Windows 7 or higher. The core project in Visual C++ interacts with the Windows API to intercept and substitute keystrokes according to a configuration file in XML. The GUI is written in C# WPF, and is used to create the configuration file that contains the layouts for the different registered keyboards.    
TODO.

# Build and Test
TODO: Describe and show how to build your code and run the tests. 

# Contribute
TODO: Explain how other users and developers can contribute to make your code better. 

If you want to learn more about creating good readme files then refer the following [guidelines](https://www.visualstudio.com/en-us/docs/git/create-a-readme). You can also seek inspiration from the below readme files:
- [ASP.NET Core](https://github.com/aspnet/Home)
- [Visual Studio Code](https://github.com/Microsoft/vscode)
- [Chakra Core](https://github.com/Microsoft/ChakraCore)

[AutoHotKey]: https://autohotkey.com/
[MSKLC]: https://msdn.microsoft.com/en-us/globalization/keyboardlayouts.aspx
[Blecha on Remapping]: https://www.codeproject.com/Articles/716591/Combining-Raw-Input-and-keyboard-Hook-to-selective