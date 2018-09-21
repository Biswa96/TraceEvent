# TraceEvent

Trace events in real time sessions with documented Windows API. The undocumented section will be added in [NtTrace repository](https://github.com/Biswa96/NtTrace.git).

## What is Event Tracing

See this Microsoft Documentation: [Event Tracing][1] 

## How to use

Download the executable from [Release Page][2]. Run this program as administrator every time. Here are the options. 

### Start a session

Run this command as administrator: `TraceEvent.exe start <Session Name> <Event Provider GUID>`. Always use an unique session name otherwise this will show error. Event provider GUIDs can be found from this Powershell cmdlet: `Get-EtwTraceProvider`. Also the [TraceLog tool][3] from Windows Driver Kit provides a list of those GUIDs and the required command: `tracelog -enumguid`. **Always use curly brackets** to specify GUID strings. Find more GUIDs in [**Event Providers list**](Event_Providers.md). For example: `TraceEvent.exe start MyTrace {12345678-1234-1234-1234-123457890ABCD}`

### Log events from a session

Run this command as administrator: `TraceEvent.exe log <Session Name>`. Only use session names which are started previously. If CPU usage becomes high redirect output to a file to logs events, e.g. `TraceEvent.exe log MyTrace > FileName.txt`. 

### Stop a session

Run this command as administrator: `TraceEvent.exe stop <Session Name>`. Stop only the previously opened tracing session. Using an already stopped session will show error. For example user this command to stop previously opened 'MyTrace' session: `TraceEvent.exe stop MyTrace`. 

## How to build

Clone this repository with `git clone https://github.com/Biswa96/TraceEvent.git`. Open the solution file TraceEvent.sln in visual Studio and build it. Or use MSBuild: `MSBuild.exe TraceEvent.sln /p:Configuration=Release`. 

makefile will be added soon. 

## Known issues

* There are some logging format issue with the Event Providers from Explorer.exe. To view those use TraceLog.exe or TraceFmt.exe from Windows Driver Kit. Use event providers from Explorer.exe carefully. 

## Further Readings

* [Event Tracing for Windows (ETW)][5]
* [Retrieving Event Data Using TDH][6]
* [Configuring and Starting an Event Tracing Session][7]

## License

This project is licensed under [GPLv3+](LICENSE). This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.

```
TraceEvent - Trace and Log Events in real time mode
Copyright (C) Biswapriyo Nath

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
