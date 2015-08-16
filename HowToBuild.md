# Introduction #

> The project was compiled and on Vista and Windows 7 machines. Using it windows versions prior to Vista is impossible due to extensive usage of TLS storage by logging infrastructure and KentCSP++ kernel. One may read more on the topic [here](http://www.nynaeve.net/?p=187). Though the OS and IDE is proprietary, the project itself uses only open source third party libraries.

Please report any problem to [borisusun](mailto:borisusun@gmail.com), if you are experiencing problems compiling the project on other windows version, or you are interested to port it to a different platform.


**I know that currently setting up an development environment requires a lot of effort. the main reason for that is a lot of dependencies that every open source project brings with it :(. Any suggestion on how to simplify building process will be warmly accepted**

# Dependencies #

The library depends on

  1. [boost 1.34.1](http://www.boostpro.com/boost_1_34_1_setup.exe) - Pre-built libraries are available to download. Download it,build, and update your IDE include and library options.
  1. [Microsoft® DirectX® 9.0 SDK](http://www.microsoft.com/downloads/details.aspx?FamilyID=77960733-06e9-47ba-914a-844575031b81&displaylang=en&Hash=CcIBpQJsJa1DOXc2imj%2bmIpBqkTgxLUszkebWkoLZxD7K5rpGv%2brjs0G6OSqxRApDNx2XdMe43273oj%2fW2PtQw%3d%3d) and its  [Extras](http://www.microsoft.com/downloads/details.aspx?FamilyID=8af0afa9-1383-44b4-bc8b-7d6315212323&displaylang=en&Hash=p2iTo8GEjAa7FfiDPjtSprxBZdgjczobuXQ65S28bJfndIx0RvfEZ9WhLP7Lut7a36MERsdyq3P5r1gbr6VkXA%3d%3d)- mediastreamer2 needs this to compile. .Download it and update your IDE include and library options.

These are pretty big dependencies well over 100Mb in complete installation each. Obviously I couldn't include them into subversion tree.

Other project dependencies:-
  1. [C++CSP](http://www.cs.kent.ac.uk/projects/ofa/c++csp/doc/installation.html)
  1. [lua](http://www.lua.org)
  1. [resiprocate 1.4.1](http://www.resiprocate.org)
  1. [json spirit](http://www.codeproject.com/KB/recipes/JSON_Spirit.aspx)
  1. [mediastreamer2](http://www.linphone.org/index.php/eng/code_review/mediastreamer2)
  1. [live555](http://www.live555.com/)
  1. [unimrcp](http://www.unimrcp.org/)

are either included into subversion tree completely or by means of subversion externals.

# Setup #

  * Checkout altalena code in '<project root>\altalena' - see instructions [here](http://code.google.com/p/altalena/source/checkout). Write to administrator if you want to become a member of development team.

  * Run "configure.bat" ensure that it prints SUCCESS after run.

  * Open <root project>\solutions\altalena\_all\altalena\_all.sln.

  * build unimrcp-deps first

  * build 'iw\_setup' project

# Start #

You can use dummy csharpio for compiliing .NET files or iw\_lualib.

For both you will have to configure the solution directory

Configuration file is located in '<project root>\iw\_run\conf.json'. Test script is located in '<project root>\altalena\iw\_run\scripts\myscript.lua'. Inspect the files and change them accordingly to your environment.

All files needed to run are copied to '<project root>\solutions\altalena\_all\<compile configuration>' directory.


Configure your iw\_lualib project to use these files for debugging.

Should look something like this:-

![http://www.prettythingsflying.com/images/debug.gif](http://www.prettythingsflying.com/images/debug.gif)

Enjoy

