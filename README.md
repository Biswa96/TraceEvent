# TraceEvent

[![Licence](https://img.shields.io/github/license/Biswa96/TraceEvent.svg?style=for-the-badge)](https://www.gnu.org/licenses/gpl-3.0.en.html)
[![Top Language](https://img.shields.io/github/languages/top/Biswa96/TraceEvent.svg?style=for-the-badge)](https://github.com/Biswa96/TraceEvent.git)
[![Code size](https://img.shields.io/github/languages/code-size/Biswa96/TraceEvent.svg?style=for-the-badge)]()

Trace Events with real time sessions using (un)documented Windows APIs and NT APIs.

## What is Event Tracing

See this Microsoft Documentation: [Event Tracing](https://docs.microsoft.com/en-us/windows/desktop/etw/event-tracing-portal)

## How to build

Clone this repository. Open the solution (.sln) or project (.vcxproj) file in Visual Studio and build it. Alternatively, run Visual Studio developer command prompt, go to the cloned folder and run `msbuild` command. You can also build with mingw-w64 toolchain. Go to the folder in terminal run `make` command for mingw-w64/msys2. 

## How to use

Download the executable from [Release Page](https://github.com/Biswa96/TraceEvent/releases). Run this program as administrator every time. Here are the options. 

```
Usage: TraceEvent.exe [--] [option] [argument]
Options:

    -E,  --enumguidinfo                      Enumerate registered trace GUIDs with all PID and Logger ID. 
    -e,  --enumguid                          Enumerate registered trace GUIDs. 
    -g,  --guid        <ProviderGUID>        Add Event Provider GUID with trace session. 
    -L,  --list                              List all registered trace sessions with details. 
    -l,  --log         <LoggerName>          Log events in real time. 
    -q,  --query       <LoggerName>          Query status of <LoggerName> trace session. 
    -S,  --start       <LoggerName>          Starts the <LoggerName> trace session. 
    -s,  --stop        <LoggerName>          Stops the <LoggerName> trace session. 
    -h,  --help                              Display this usage information. 
```

### Start a session

Run this command as administrator: `TraceEvent.exe --start <Session Name> --guid <Event Provider GUID>`. Always use an unique session name otherwise this will show error. Event provider GUIDs can be found from this Powershell cmdlet: `Get-EtwTraceProvider`. **Always use curly brackets** to specify GUID strings. Find more GUIDs in [**Event Providers list**](docs/Event_Providers.md). For example: `TraceEvent.exe --start MyTrace --guid {12345678-1234-1234-1234-123457890ABCD}`

### Log events

Run this command as administrator: `TraceEvent.exe --log <Session Name>`. Only use session names which are started previously. If CPU usage becomes high then redirect output to a file. e.g. `TraceEvent.exe --log MyTrace > FileName.txt` 

### Stop a session

Run this command as administrator: `TraceEvent.exe --stop <Session Name>`. Stop only the previously opened tracing session. Using an already stopped session will show error. For example user this command to stop previously opened 'MyTrace' session: `TraceEvent.exe --stop MyTrace`. 

## Project Overview

Here are the overview of source files according to their dependencies:

```
TraceEvent\
    |
    +-- WinInternal: Crafted TRACE_CONTROL_FUNCTION_CLASS and NT API's definitions
    +-- PrintProperties: Display Event session details and it's security properties
    +-- CallBacks: Callback functions to log events messages
        |
        |   +-- Log: Helper functions to Log status and convert GUID to string
        |   +-- Helpers: Helper/Auxiliary functions for SecHost functions
        |   +-- SecHost: Internal functions from SecHost.dll, Advapi32.dll etc.
        |   |
        +-- TraceEvent: Functions to start, stop, log and other tasks
            |
            |    +-- wgetopt: Converted from Cygwin getopt file for wide characters
            |    |
            +-- main: Main function with option processing
```

## Further Readings

* [Event Tracing for Windows (ETW)](https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/event-tracing-for-windows--etw-) 
* [Retrieving Event Data Using TDH](https://docs.microsoft.com/en-us/windows/desktop/etw/retrieving-event-data-using-tdh) 
* [Configuring and Starting an Event Tracing Session](https://docs.microsoft.com/en-us/windows/desktop/etw/configuring-and-starting-an-event-tracing-session) 

## Acknowledgments

Thanks to:

* ProcessHacker's collection of [native API header file](https://github.com/processhacker/processhacker/tree/master/phnt) 
* wbenny's [pedbex](https://github.com/wbenny/pdbex) tool 
* RedPlait Blog: [NtTraceControl](https://redplait.blogspot.com/2011/02/nttracecontrol.html) 
* Geoff Chappell: [NtTraceControl](http://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/etw/traceapi/control/index.htm) 

## License

This project is licensed under [GPLv3+](LICENSE). This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.

```
TraceEvent -- (c) Copyright 2018-19 Biswapriyo Nath

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

<!-- END of README -->
