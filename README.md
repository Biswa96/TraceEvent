# TraceEvent

[![Licence](https://img.shields.io/github/license/Biswa96/TraceEvent.svg?style=for-the-badge)](https://www.gnu.org/licenses/gpl-3.0.en.html)
[![Top Language](https://img.shields.io/github/languages/top/Biswa96/TraceEvent.svg?style=for-the-badge)](https://github.com/Biswa96/TraceEvent.git)
[![Code size](https://img.shields.io/github/languages/code-size/Biswa96/TraceEvent.svg?style=for-the-badge)]()

Trace events in real time sessions with documented Windows API. The undocumented section will be added in second version.

## What is Event Tracing

See this Microsoft Documentation: [Event Tracing][1] 

## How to build

Clone this repository. Open the solution (.sln) or project (.vcxproj) file in Visual Studio and build it. Alternatively, run Visual Studio developer command prompt, go to the cloned folder and run this command: `msbuild.exe /p:Configuration=Debug`. You can also build with mingw-w64 toolchain. Go to the folder in terminal run `make` command for mingw-w64/msys2/cygwin. 

## How to use

Download the executable from [Release Page][2]. Run this program as administrator every time. Here are the options. 

```
Usage: TraceEvent.exe [--] [option] [argument]
Options:
    -g,  --guid    <ProviderGUID>        Add Event Provider GUID with trace session.
    -L,  --list                          List all trace sessions.
    -l,  --log     <LoggerName>          Log events in real time.
    -q,  --query   <LoggerName>          Query status of <LoggerName> trace session.
    -S,  --start   <LoggerName>          Starts the <LoggerName> trace session.
    -s,  --stop    <LoggerName>          Stops the <LoggerName> trace session.
    -h,  --help                          Display usage information.
```

### Start a session

Run this command as administrator: `TraceEvent.exe --start <Session Name> --guid <Event Provider GUID>`. Always use an unique session name otherwise this will show error. Event provider GUIDs can be found from this Powershell cmdlet: `Get-EtwTraceProvider`. Also the [TraceLog tool][3] from Windows Driver Kit provides a list of those GUIDs and the required command: `tracelog -enumguid`. **Always use curly brackets** to specify GUID strings. Find more GUIDs in [**Event Providers list**](Event_Providers.md). For example: `TraceEvent.exe --start MyTrace --guid {12345678-1234-1234-1234-123457890ABCD}`

### Log events

Run this command as administrator: `TraceEvent.exe --log <Session Name>`. Only use session names which are started previously. If CPU usage becomes high then redirect output to a file. e.g. `TraceEvent.exe --log MyTrace > FileName.txt` 

### Stop a session

Run this command as administrator: `TraceEvent.exe --stop <Session Name>`. Stop only the previously opened tracing session. Using an already stopped session will show error. For example user this command to stop previously opened 'MyTrace' session: `TraceEvent.exe --stop MyTrace`. 

## Project Overview

Here are the overview of source files according to their dependencies:

```
TraceEvent\
    |
    +-- Functions: Helper functions to Log status and convert GUID to string
    +-- PrintProperties: Display Event session details and it's security properties
    +-- CallBacks: Callback functions to log events messages
    +-- TraceEvent: Functions to start, stop, log and other tasks
        |
        |   +-- wgetopt: Converted from Cygwin getopt file for wide characters
        |   |
        +-- main: Main function with option processing
```

## Further Readings

* [Event Tracing for Windows (ETW)][5]
* [Retrieving Event Data Using TDH][6]
* [Configuring and Starting an Event Tracing Session][7]

## License

This project is licensed under [GPLv3+](LICENSE). This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.

```
TraceEvent -- (c) Copyright 2018 Biswapriyo Nath

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

<!-- Links -->
[1]: https://docs.microsoft.com/en-us/windows/desktop/etw/event-tracing-portal
[2]: https://github.com/Biswa96/TraceEvent/releases
[3]: https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/tracelog
[4]: https://docs.microsoft.com/en-us/windows/desktop/etw/logging-mode-constants
[5]: https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/event-tracing-for-windows--etw-
[6]: https://docs.microsoft.com/en-us/windows/desktop/etw/retrieving-event-data-using-tdh
[7]: https://docs.microsoft.com/en-us/windows/desktop/etw/configuring-and-starting-an-event-tracing-session

<!-- END of README -->
