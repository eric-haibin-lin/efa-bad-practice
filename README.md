Usage:
## compile code 
```
mkdir build && cd build && cmake .. && make 

```

## run 
```
# server
./msg_serv 127.0.0.1 8888 1
./nothd_serv 127.0.0.1 8888 1

# client
./msg_cli 127.0.0.1 8888 1
./nothd_cli 127.0.0.1 8888 1

```
## issue
efa-install 1.8.0
single node:
- Recv bw: 31.5297 Gbps
- Send bw: 31.3995 Gbps
