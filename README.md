# WARNING
- ***This project is still in devlopment! Dont use in production!***
- ***I am not responsible to damages that accours if production is damaged!***

# All For All
- Is an opensource tool library for os-admins
- Is written in C++ for blazing fast applications and code readibility. So, other people can join.
- Is a massive toolkit (SOON...)

## alltools
- Binary tools for quick code writing in bash or batch
- Written in cpp

### Nothing tool
- The nothing tools does... as you guessed nothing
- The normal mode:
```bash
nothing
```
	* Does nothing.
	* Thats it, don't expect more.
	
- Sleeping mode:
```bash
nothing -s 1000 # Sleeps for 1 second or 1000 Milliseconds.
```
	* Sleeps while doing nothing
	* Thats it, again.
	
### Lockout tool
- Logout from a machine
- the normal mode:
```bash
lockout # Logout
```
	* Logs out of the machine with specified API of the platform
	* Cross-platform
- Sleep mode:
```bash
lockout -s 1000 # Sleep for 1Second or 1000ms
```
	* Sleeps for specified time until logout

### Serve tool
- Create a server from a machine
- Start a server at 0.0.0.0:8080/ from the . folder
```bash
serve
```
	* Serves files from 8080 from the . folder
```bash
serve --port 443 --cors --https --folder \"servethething\"
```
	* serves from port 443
	* adds CORS Headers
	* uses https SSL
	* serves from the servethething folder