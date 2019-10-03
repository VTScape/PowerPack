NI server -- binary that runs on Windows

If we want to want run another GPU, we have to copy paste the binary, 
change the config, and that'll act as another binary.

You have to connect your machine to the DORI cluster **before** connection ot the linux box or Windows.



Connect to the linux box (ip: 172.16.0.240)
Linux:
```
ssh n240 
```


metertools_4.0_n11_s240_20130409/src/tool/mcontrol

All the server daemons are in `metertools/src/tools/mcontrol`


## mserver and multiserver
---
1. Start mserver

before starting mserver run `sudo nohup ./mserver -s /dev/ttyUSB0 -o -p 6911 &` on the linux box.

	-s <usbportname> (Port that the meter is connected to e.g. /dev/ttyUSB0)
	-o Output the value read to standard out
	-p <port number assigned to service>

>Port naming convention
>			69 - port
>			11 - node number

windows and linux box will talk over that port
> There can be multiple usbports on a node

	2)starting multiserver
	./multiserver &

	power pac is running for the GPU machine!!!!!!

fcontrol.h



results
	col 1 cpu
	col 2 memory
	col 3 user disc or motherboard(?) 
	col 4 user disc or motherboard(?)

	every line is 1 timestamp

	<COL1> <COL2> <COL3> <COL4> || <POWER MEASUREMENTS OF EACH RAIL>
	power measurements are aggregated into the first four columns.
	We can change the aggregation, but we need to recompile afterwards



have to use the multimeter to measure what each rail is assigned to
--run a benchmark targeting a specific rail and see which one is poppin' off


|wserver|
project folder

ContAcq-IntClk.c
--once hardware is done we have to record the voltage information into this file


vim multiserver.c
#define DEFAULT_METER_PORT <-- will have to change when setting up a new machine



P.S.
sometimes for the gpu machine, we can't map the rail to a single component
e.g. you see the same rail powering up on different benchmarks


needs PCIe riser cards
